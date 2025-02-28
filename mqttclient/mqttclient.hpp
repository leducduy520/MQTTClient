#ifndef __CORE_MQTT_CLIENT__
#define __CORE_MQTT_CLIENT__
#include "mqtt/async_client.h"
#include "types.hpp"
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>

const unsigned int QOS = 1;
const auto TIMEOUT = std::chrono::seconds(5);
const size_t MAX_MESSAGE_STACK_SIZE = 1024;

namespace mqttcpp {
enum class CallbackEvent {
  CONNECTED,
  DISCONNECTED,
  CONNECTION_LOST,
  CONNECTION_UPDATE,
  MESSAGE_ARRIVED,
  DELIVERY_COMPLETE,
  ACTION_SUCCESS,
  ACTION_FAILURE
};

class MqttClient {
  using lg = std::lock_guard<std::mutex>;

  inline void set_default_handler() {
    client_.set_connected_handler([this](const mqtt::string &cause) {
      this->self_handle_callback_event(CallbackEvent::CONNECTED, cause);
    });
    client_.set_connection_lost_handler([this](const mqtt::string &cause) {
      this->self_handle_callback_event(CallbackEvent::CONNECTION_LOST, cause);
    });
    client_.set_disconnected_handler(
        [this](const mqtt::properties &props, mqtt::ReasonCode reason) {
          this->self_handle_callback_event(CallbackEvent::DISCONNECTED,
                                           disconnect_data{props, reason});
        });
    client_.set_update_connection_handler([this](mqtt::connect_data &data) {
      this->self_handle_callback_event(CallbackEvent::CONNECTION_UPDATE, {});
      return true;
    });
    client_.set_message_callback([this](mqtt::const_message_ptr msg) {
      this->self_handle_callback_event(CallbackEvent::MESSAGE_ARRIVED, msg);
    });
  }

  inline void make_wait(mqtt::token_ptr token, unsigned int wait_for) {
    if (wait_for > 0) {
      token->wait_for(wait_for);
    } else {
      token->wait();
    }
  }

  bool common_try(std::function<void()> fn, const char *fnId);
  bool consume_message(bool allow);

  mqtt::connect_options connOpts_;
  std::unique_ptr<mqtt::iaction_listener> pubListener_;
  std::unique_ptr<mqtt::iaction_listener> subListener_;
  std::unique_ptr<mqtt::iaction_listener> connListener_;
  std::unique_ptr<mqtt::iaction_listener> disconnListener_;

  std::mutex consumeGuard_;
  std::condition_variable cv_;
  std::atomic<bool> consumeFlag_;

protected:
  friend class DefaultActionListener;

  virtual void self_handle_callback_event(CallbackEvent event,
                                          CallbackVariant info);

  mqtt::async_client client_;
  std::function<void(CallbackEvent, CallbackVariant)> exteventHandler_;
  exception_trace_ptr excPtr_;

public:
  MqttClient(const std::string &serverAddress, const std::string &clientId);
  MqttClient(const std::string &serverAddress, const std::string &clientId,
             mqtt::connect_options connectOptions);
  MqttClient(const std::string &serverAddress, const std::string &clientId,
             mqtt::create_options createOptions,
             mqtt::connect_options connectOptions);

  virtual ~MqttClient();

  void set_connOpts(const mqtt::connect_options opts);
  void set_connOpts(mqtt::connect_options &&opts);
  void set_exception_trace(exception_trace_ptr ptr);
  void set_event_handler(
      std::function<void(CallbackEvent, CallbackVariant)> handler);

  bool connect(bool wait = true, unsigned int wait_for = 0);
  bool connect(mqtt::token_ptr &token);

  bool disconnect(bool wait = true, unsigned int wait_for = 0);
  bool disconnect(mqtt::token_ptr &token);

  bool subscribe(mqtt::token_ptr &token, const std::string &topic,
                 unsigned int qos = QOS);
  bool subscribe(const std::string &topic, unsigned int qos = QOS,
                 bool wait = true, unsigned int wait_for = 0);

  bool publish(mqtt::token_ptr &token, const std::string &topic,
               const std::string &payload, unsigned int qos);
  bool publish(const std::string &topic, const std::string &payload,
               unsigned int qos = QOS, bool wait = true,
               unsigned int wait_for = 0);

  bool connected() const;

  bool start_consume();
  bool stop_consume();
  bool pop_message(mqtt::binary &msg);
  bool is_consuming() const;
  static std::unique_ptr<MqttClient> Instance;
};

class DefaultActionListener : public mqtt::iaction_listener {
  MqttClient *parent_;

public:
  DefaultActionListener(class MqttClient *parent);
  void on_failure(const mqtt::token &asyncActionToken) override;
  void on_success(const mqtt::token &asyncActionToken) override;
};
} // namespace mqttcpp

#endif // __CORE_MQTT_CLIENT__