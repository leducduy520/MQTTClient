#ifndef __CORE_MQTT_CLIENT__
#define __CORE_MQTT_CLIENT__
#include <string>
#include <chrono>
#include <memory>
#include <mutex>
#include <atomic>
#include "mqtt/async_client.h"
#include "types.hpp"

namespace mqttcpp
{
    enum class CallbackEvent
    {
        EVENT_CONNECTED,         ///< Event received from client when connected to broker.
        EVENT_DISCONNECTED,      ///< Event received from client when disconnected from broker
        EVENT_CONNECTION_LOST,   ///< Event received from client when connection to broker is lost.
        EVENT_CONNECTION_UPDATE, ///< Event received from client when connection data is updated.
        EVENT_MESSAGE_ARRIVED,   ///< Event received from client when a message arrives from broker.
        EVENT_DELIVERY_COMPLETE, ///< Event received from client when message delivery is complete.
        EVENT_ACTION_SUCCESS,    ///< Event received from client when an action is successful.
        EVENT_ACTION_FAILURE     ///< Event received from client when an action fails.
    };

    class MqttClient
    {
        using lg = std::lock_guard<std::mutex>;

        /**
         * @brief Sets the default handlers for various MQTT client events.
         *
         * This function sets up the following handlers for the MQTT client:
         * - Connected handler: Invoked when the client successfully connects to the broker.
         * - Connection lost handler: Invoked when the connection to the broker is lost.
         * - Disconnected handler: Invoked when the client is disconnected from the broker.
         * - Update connection handler: Invoked to update the connection data.
         * - Message callback: Invoked when a message arrives from the broker.
         *
         * Each handler calls the `self_handle_callback_event` method with the appropriate
         * `CallbackEvent` and data.
         * @sa self_handle_callback_event
         */
        inline void set_default_handler()
        {
            client_.set_connected_handler(

                [this](const mqtt::string& cause) {
                    this->self_handle_callback_event(CallbackEvent::EVENT_CONNECTED, cause);
                });
            client_.set_connection_lost_handler(

                [this](const mqtt::string& cause) {
                    this->self_handle_callback_event(CallbackEvent::EVENT_CONNECTION_LOST, cause);
                });
            client_.set_disconnected_handler([this](const mqtt::properties& props, mqtt::ReasonCode reason) {
                this->self_handle_callback_event(CallbackEvent::EVENT_DISCONNECTED, disconnect_data{props, reason});
            });
            client_.set_update_connection_handler([this](mqtt::connect_data& data) {
                this->self_handle_callback_event(CallbackEvent::EVENT_CONNECTION_UPDATE, data);
                return true;
            });
            client_.set_message_callback([this](mqtt::const_message_ptr msg) {
                this->self_handle_callback_event(CallbackEvent::EVENT_MESSAGE_ARRIVED, msg);
            });
        }

        /**
         * @brief Waits for the MQTT token to complete.
         *
         * @param token The MQTT token to wait for.
         * @param wait_for The duration to wait for the token to complete, in milliseconds.
         *                 If zero or negative, waits indefinitely.
         */
        inline void make_wait(mqtt::token_ptr token, unsigned int wait_for)
        {
            if (wait_for > 0)
            {
                token->wait_for(wait_for);
            }
            else
            {
                token->wait();
            }
        }

        /**
         * @brief Attempts to execute a given function and handles any exceptions.
         *
         * @param fn The callable object to be executed.
         * @param fnId A string identifier for the function, used for logging or debugging purposes.
         * @return true if the function executed without throwing an exception, false otherwise.
         */
        bool common_try(std::function<void()> fn, const char* fnId);

        /**
         * @brief Consumes a message from the MQTT client.
         *
         * This function attempts to consume a message from the MQTT client. The behavior
         * of the function can be controlled by the `allow` parameter.
         *
         * @param allow A boolean flag that determines whether the function is allowed
         *              to consume a message. If `true`, the function will attempt to
         *              consume a message. If `false`, the function will not consume
         *              any message.
         * @return Returns `true` if a message was successfully consumed, `false` otherwise.
         */
        bool consume_message(bool allow);

    protected:
        friend class DefaultActionListener;

        mqtt::connect_options connOpts_;                          ///< Connection options for the MQTT client.
        std::unique_ptr<mqtt::iaction_listener> pubListener_;     ///< Listener for publish actions.
        std::unique_ptr<mqtt::iaction_listener> subListener_;     ///< Listener for subscribe actions.
        std::unique_ptr<mqtt::iaction_listener> unsubListener_;   ///< Listener for unsubscribe actions.
        std::unique_ptr<mqtt::iaction_listener> connListener_;    ///< Listener for connection actions.
        std::unique_ptr<mqtt::iaction_listener> disconnListener_; ///< Listener for disconnection actions.

        std::mutex consumeGuard_;       ///< Mutex for guarding the message consumption.
        std::condition_variable cv_;    ///< Condition variable for message consumption.
        std::atomic<bool> consumeFlag_; ///< Flag to control message consumption.

        mqtt::async_client client_;                                           ///< Client object for the MQTT client.
        std::function<void(CallbackEvent, CallbackVariant)> exteventHandler_; ///< External event handler callback.
        exception_trace_ptr excPtr_; ///< Pointer to the last exception that was caught.

        /**
         * @brief Handles a callback event.
         *
         * This virtual function processes a callback event indicated by the provided event type
         * along with its associated variant information.
         *
         * @param event The callback event type that specifies the kind of event to handle.
         * @param info The associated variant information containing event-specific data.
         */
        virtual void self_handle_callback_event(CallbackEvent event, CallbackVariant info);

    public:
        MqttClient(const std::string& serverAddress, const std::string& clientId);
        MqttClient(const std::string& serverAddress, const std::string& clientId, mqtt::connect_options connectOptions);
        MqttClient(const std::string& serverAddress,
                   const std::string& clientId,
                   mqtt::create_options createOptions,
                   mqtt::connect_options connectOptions);

        virtual ~MqttClient();

        /**
         * @brief Sets the MQTT connection options.
         *
         * @param opts The MQTT connection options to be applied.
         */
        void set_connOpts(const mqtt::connect_options opts);

        /**
         * @brief Sets the event handler callback.
         *
         * This function registers a callback function that will be invoked when a specific event occurs.
         * The callback receives a CallbackEvent indicating the type of event and a CallbackVariant carrying
         * associated data related to the event.
         *
         * @param handler A std::function that takes a CallbackEvent and a CallbackVariant, defining the behavior 
         *                to be executed when an event occurs.
         */
        void set_event_handler(std::function<void(CallbackEvent, CallbackVariant)> handler);

        inline void unset_event_handler()
        {
            exteventHandler_ = nullptr;
        }

        /**
         * @brief Establishes a connection to the MQTT broker.
         *
         * This function attempts to connect to the MQTT broker using the provided token.
         *
         * @param token A reference to a shared pointer of type mqtt::token_ptr that will be used for the connection.
         * @return true if no error occurs; false otherwise.
         */
        bool connect(mqtt::token_ptr& token);

        /**
         * @brief Connect to the MQTT broker.
         *
         * Optionally waits until the connection is complete or times out.
         *
         * @param wait If true, blocks until the operation completed or the timeout expires.
         * @param wait_for Maximum time (in milliseconds) to wait. A value of 0 indicates infinite timeout.
         * @return true if no error occurs; false otherwise.
         */
        bool connect(bool wait = true, unsigned int wait_for = 0);

        /**
         * @brief Disconnects the MQTT client.
         * 
         * This function initiates the disconnection process for the MQTT client.
         * 
         * @param token A reference to a shared pointer of the MQTT token that will be used for the disconnection process.
         * @return true if the disconnection was successful, false otherwise.
         */
        bool disconnect(mqtt::token_ptr& token);

        /**
         * @brief Disconnects the MQTT client from the server.
         *
         * Optionally waits until the disconnection is complete or times out.
         *
         * @param wait If true, blocks until the operation completed or the timeout expires.
         * @param wait_for Maximum time (in milliseconds) to wait. A value of 0 indicates infinite timeout.
         * @return true if no error occurs; false otherwise.
         */
        bool disconnect(bool wait = true, unsigned int wait_for = 0);

        /**
         * @brief Subscribes to a specified MQTT topic with a given Quality of Service (QoS) level.
         *
         * @param token A reference to an MQTT token pointer that will be used for the subscription.
         * @param topic The topic to which the client will subscribe.
         * @param qos The Quality of Service level for the subscription (0, 1, or 2).
         * @return true if no error occurs; false otherwise.
         */
        bool subscribe(mqtt::token_ptr& token, const std::string& topic, unsigned int qos);

        /**
         * @brief Subscribes to a specified MQTT topic.
         *
         * This method sends a subscribe request for the given topic with a specific Quality of Service (QoS) level.
         * It optionally waits for the subscription acknowledgement based on the provided wait parameter.
         *
         * @param topic The MQTT topic to subscribe to.
         * @param qos The Quality of Service level for the subscription (defaults to QOS).
         * @param wait If true, blocks until the operation completed or the timeout expires.
         * @param wait_for Maximum time (in milliseconds) to wait. A value of 0 indicates infinite timeout.
         *
         * @return true if no error occurs; false otherwise.
         */
        bool subscribe(const std::string& topic, unsigned int qos = 1, bool wait = true, unsigned int wait_for = 0);

        /**
         * @brief Unsubscribes from a given MQTT topic.
         * 
         * This function unsubscribes the client from the specified MQTT topic.
         * 
         * @param token A reference to the MQTT token pointer associated with the unsubscribe request.
         * @param topic The topic string from which to unsubscribe.
         * @return true if no error occurs; false otherwise.
         */
        bool unsubscribe(mqtt::token_ptr& token, const std::string& topic);

        /**
         * @brief Unsubscribes from the specified MQTT topic.
         *
         * This function initiates an unsubscribe operation on the given topic.
         * Optionally, it can wait for the operation to complete based on the provided parameters.
         *
         * @param topic The MQTT topic to unsubscribe from.
         * @param wait If true, blocks until the operation completed or the timeout expires.
         * @param wait_for Maximum time (in milliseconds) to wait. A value of 0 indicates infinite timeout.
         * @return true if no error occurs; false otherwise.
         */
        bool unsubscribe(const std::string& topic, bool wait = true, unsigned int wait_for = 0);

        /**
         * @brief Publishes a message to a specified MQTT topic.
         * 
         * @param token A reference to an MQTT token pointer that will be used for the publish operation.
         * @param topic The topic to which the message will be published.
         * @param payload The message payload to be published.
         * @param qos The Quality of Service level for the message delivery (default is QOS).
         * @return true if no error occurs; false otherwise.
         */
        bool publish(mqtt::token_ptr& token,
                     const std::string& topic,
                     const std::string& payload,
                     unsigned int qos = 1);

        /**
         * @brief Publishes a message to an MQTT topic.
         *
         * This function sends a message with the specified payload to the given MQTT topic.
         * The Quality of Service (QoS) level is configurable through the qos parameter.
         * Optionally, it can wait for an acknowledgment or confirmation after publishing,
         * as specified by the wait and wait_for parameters.
         *
         * @param topic The MQTT topic where the message is to be published.
         * @param payload The content of the message to be sent.
         * @param qos The Quality of Service level for the message delivery. Defaults to QOS.
         * @param wait If true, blocks until the operation completed or the timeout expires.
         * @param wait_for Maximum time (in milliseconds) to wait. A value of 0 indicates infinite timeout.
         *
         * @return true if no error occurs; false otherwise.
         */
        bool publish(const std::string& topic,
                     const std::string& payload,
                     unsigned int qos = 1,
                     bool wait = true,
                     unsigned int wait_for = 0);

        /**
         * @brief Retrieves the last exception that was thrown.
         * 
         * This function returns a pointer to the last exception that was caught and stored.
         * 
         * @return exception_trace_ptr A pointer to the last exception.
         */
        inline exception_trace_ptr get_last_exception() const
        {
            return excPtr_;
        }

        /**
         * @brief Determines if the MQTT client is currently connected.
         *
         * @return True if the client is connected; false if it is not.
         */
        bool connected();

        /**
         * @brief Reconnects the MQTT client
         * 
         */
        inline void reconnect()
        {
            client_.reconnect();
        }

        /**
         * @brief Starts saving messages.
         *
         * @return true if saving messages is successfully started; false otherwise.
         * @sa consume_message
         */
        inline bool start_saving_message()
        {
            return this->consume_message(true);
        }

        /**
         * @brief Stops saving messages.
         *
         * @return true if the operation succeeds, false otherwise.
         */
        inline bool stop_saving_message()
        {
            return this->consume_message(false);
        }

        /**
         * @brief Checks whether the saving of the message is active.
         *
         * @return True if the message is being saved, otherwise false.
         */
        inline bool is_saving_message() const
        {
            return consumeFlag_.load();
        }

        /**
         * @brief Retrieves the next available MQTT binary message.
         *
         * @param msg A reference to an mqtt::binary object that will be populated with the message data.
         * @return true if a message was successfully retrieved, false otherwise.
         */
        bool get_next_message(mqtt::binary& msg);

        static std::unique_ptr<MqttClient> Instance;
    };

    /**
     * @brief Default action listener for MQTT client actions.
     *
     * This class provides default implementations for handling the success and failure
     * of asynchronous MQTT client actions. It forwards the events to the parent MqttClient
     * instance for further processing.
     */
    class DefaultActionListener : public mqtt::iaction_listener
    {
        MqttClient* parent_; ///< Pointer to the parent MqttClient instance.

    public:
        /**
         * @brief Constructs a DefaultActionListener with a parent MqttClient.
         *
         * @param parent Pointer to the parent MqttClient instance.
         */
        DefaultActionListener(class MqttClient* parent);

        /**
         * @brief Called when an asynchronous action fails.
         *
         * This method is invoked when an asynchronous MQTT action fails. It forwards
         * the failure event to the parent MqttClient instance.
         *
         * @param asyncActionToken The token associated with the failed action.
         */
        void on_failure(const mqtt::token& asyncActionToken) override;

        /**
         * @brief Called when an asynchronous action succeeds.
         *
         * This method is invoked when an asynchronous MQTT action succeeds. It forwards
         * the success event to the parent MqttClient instance.
         *
         * @param asyncActionToken The token associated with the successful action.
         */
        void on_success(const mqtt::token& asyncActionToken) override;
    };
} // namespace mqttcpp

#endif // __CORE_MQTT_CLIENT__