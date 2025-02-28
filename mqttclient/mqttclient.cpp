#include "mqttclient.hpp"
#include "mqttclient.hpp"
#include "mqttclient.hpp"
#include "mqttclient.hpp"
#include <sstream>
#include "mqttclient.hpp"
#include "monitor.hpp"

using namespace mqtt;

namespace mqttcpp
{
    std::unique_ptr<MqttClient> MqttClient::Instance{nullptr};

    static std::string mqttEventToString(CallbackEvent event)
    {
        switch (event)
        {
        case CallbackEvent::CONNECTED:
            return "CONNECTED";
        case CallbackEvent::DISCONNECTED:
            return "DISCONNECTED";
        case CallbackEvent::CONNECTION_UPDATE:
            return "CONNECTION_UPDATE";
        case CallbackEvent::CONNECTION_LOST:
            return "CONNECTION_LOST";
        case CallbackEvent::MESSAGE_ARRIVED:
            return "MESSAGE_ARRIVED";
        case CallbackEvent::DELIVERY_COMPLETE:
            return "DELIVERY_COMPLETE";
        case CallbackEvent::ACTION_SUCCESS:
            return "ACTION_SUCCESS";
        case CallbackEvent::ACTION_FAILURE:
            return "ACTION_FAILURE";
        default:
            return "UNKNOWN";
        }
    }

    DefaultActionListener::DefaultActionListener(MqttClient *parent) : parent_(parent)
    {
    }

    void DefaultActionListener::on_failure(const mqtt::token &tok)
    {
        if (parent_)
        {
            parent_->self_handle_callback_event(CallbackEvent::ACTION_FAILURE,
                                                mqtt::token::create(tok.get_type(),
                                                                    *tok.get_client(),
                                                                    tok.get_topics(),
                                                                    tok.get_user_context(),
                                                                    *tok.get_action_callback()));
        }
    }

    void DefaultActionListener::on_success(const mqtt::token &tok)
    {
        if (parent_)
        {
            parent_->self_handle_callback_event(CallbackEvent::ACTION_SUCCESS,
                                                token::create(tok.get_type(),
                                                              *tok.get_client(),
                                                              tok.get_topics(),
                                                              tok.get_user_context(),
                                                              *tok.get_action_callback()));
        }
    }

    bool MqttClient::common_try(std::function<void()> fn, const char *fnId)
    {
        try
        {
            fn();
            if (excPtr_)
            {
                excPtr_->push(false);
            }
            return true;
        }
        catch (const mqtt::exception &exc)
        {
            derror1("[MqttClient] %s error: ", fnId) << exc.what() << std::endl;
            if (excPtr_)
            {
                excPtr_->push(false, exc);
            }
        }
        catch (const std::exception &exc)
        {
            derror1("[MqttClient] %s error: Standard exception\n", fnId).print();
            if (excPtr_)
            {
                excPtr_->push(true, exc.what());
            }
        }
        catch (...)
        {
            derror1("[MqttClient] %s error: Unknown exception\n", fnId).print();
            if (excPtr_)
            {
                excPtr_->push(true);
            }
        }
        return false;
    }

    void MqttClient::self_handle_callback_event(CallbackEvent event, CallbackVariant info)
    {
        dinfo2("[MqttClient] Event ") << mqttEventToString(event) << std::endl;

        switch (event)
        {
        case CallbackEvent::CONNECTED:
        {
            std::ostringstream oss;
            oss << "Connected to broker." << std::endl;
            if (!info.asString().empty())
            {
                oss << " Cause: " << info.asString();
            }
            dinfo2(oss.str().c_str()) << std::endl;
        }
        break;
        case CallbackEvent::DISCONNECTED:
        {
            std::ostringstream oss;
            oss << "Disconnected from broker.";
            disconnect_data disconnData = info.asDisconnectData();
            if (!disconnData.props.empty())
            {
                oss << " ReasonString: " << get<string>(disconnData.props, property::REASON_STRING) << ", ReasonCode: " << disconnData.reason;
            }
            dinfo2(oss.str().c_str()) << std::endl;
        }
        break;
        case CallbackEvent::CONNECTION_UPDATE:
        {
            dinfo2("Connection update received") << std::endl;
        }
        break;
        case CallbackEvent::CONNECTION_LOST:
        {
            std::ostringstream oss;
            oss << "Connection lost.";
            if (!info.asString().empty())
            {
                oss << " Cause: " << info.asString();
            }
            dinfo2(oss.str().c_str()) << std::endl;
        }
        break;
        case CallbackEvent::MESSAGE_ARRIVED:
        {
            mqtt::const_message_ptr msg = info.asMessage();
            if (msg)
            {
                std::ostringstream oss;
                oss << "Topic: " << msg->get_topic() << ", Payload: " << msg->to_string()
                    << ", Retained: " << (msg->is_retained() ? "true" : "false");
                dinfo2("Message arrived: ") << oss.str() << std::endl;
            }
        }
        break;
        case CallbackEvent::ACTION_SUCCESS:
        {
            mqtt::token_ptr ptok = info.asToken();
            if (ptok)
            {
                std::ostringstream oss;
                oss;
                dinfo2("") << "Action " << ptok->get_type() << " success\n"
                           << ddbg::end();

                if (ptok->get_type() == mqtt::token::DISCONNECT)
                {
                    properties props;
                    props.add({property::REASON_STRING, "User has manually disconnected to brocker"});
                    this->self_handle_callback_event(CallbackEvent::DISCONNECTED, disconnect_data{props, mqtt::ReasonCode::NORMAL_DISCONNECTION});
                }
            }
        }
        break;
        case CallbackEvent::ACTION_FAILURE:
        {
            mqtt::token_ptr ptok = info.asToken();
            if (ptok)
            {
                std::ostringstream oss;
                oss << "Action " << ptok->get_type() << " fail\n";
                dinfo2(oss.str().c_str()).print();
            }
        }
        break;
        default:
            break;
        }

        if (exteventHandler_)
        {
            exteventHandler_(event, info);
        }
    }

    MqttClient::MqttClient(const std::string &serverAddress, const std::string &clientId)
        : pubListener_(new DefaultActionListener(this)), subListener_(new DefaultActionListener(this)),
          connListener_(new DefaultActionListener(this)), disconnListener_(new DefaultActionListener(this)),
          client_(serverAddress, clientId), consumeFlag_(false)
    {
        connOpts_.set_keep_alive_interval(10);
        connOpts_.set_clean_session(true);
        connOpts_.set_automatic_reconnect(true);
        connOpts_.set_connect_timeout(10);

        set_default_handler();
    }

    MqttClient::MqttClient(const std::string &serverAddress,
                           const std::string &clientId,
                           mqtt::connect_options connectOptions)
        : connOpts_(connectOptions), pubListener_(new DefaultActionListener(this)),
          subListener_(new DefaultActionListener(this)), connListener_(new DefaultActionListener(this)),
          disconnListener_(new DefaultActionListener(this)), client_(serverAddress, clientId),
          consumeFlag_(false)
    {
        set_default_handler();
    }

    MqttClient::MqttClient(const std::string &serverAddress,
                           const std::string &clientId,
                           mqtt::create_options createOptions,
                           mqtt::connect_options connectOptions)
        : connOpts_(connectOptions), pubListener_(new DefaultActionListener(this)),
          subListener_(new DefaultActionListener(this)), connListener_(new DefaultActionListener(this)),
          disconnListener_(new DefaultActionListener(this)), client_(serverAddress, clientId, createOptions),
          consumeFlag_(false)

    {
        set_default_handler();
    }

    MqttClient::~MqttClient()
    {
        consume_message(false);
    }

    void MqttClient::set_connOpts(const mqtt::connect_options opts)
    {
        connOpts_ = opts;
    }

    void MqttClient::set_connOpts(mqtt::connect_options &&opts)
    {
        connOpts_ = std::move(opts);
    }

    void MqttClient::set_exception_trace(exception_trace_ptr ptr)
    {
        excPtr_ = ptr;
    }

    void MqttClient::set_event_handler(std::function<void(CallbackEvent, CallbackVariant)> handler)
    {
        exteventHandler_ = handler;
    }

    bool MqttClient::connect(bool wait, unsigned int wait_for)
    {
        mqtt::token_ptr token;
        bool res = connect(token);
        if (res && wait)
        {
            make_wait(token, wait_for);
        }
        return res;
    }

    bool MqttClient::connect(mqtt::token_ptr &token)
    {
        std::function<void()> fn = [this, &token]() mutable
        {
            dinfo1("[MqttClient] Connecting to broker...\n").print();
            token = client_.connect(connOpts_, nullptr, *connListener_);
        };
        return common_try(fn, "Connect");
    }

    bool MqttClient::disconnect(bool wait, unsigned int wait_for)
    {
        mqtt::token_ptr token;
        bool res = disconnect(token);
        if (res && wait)
        {
            make_wait(token, wait_for);
        }
        return res;
    }

    bool MqttClient::disconnect(mqtt::token_ptr &token)
    {
        std::function<void()> fn = [this, &token]() mutable
        {
            dinfo1("[MqttClient] Disconnecting...") << std::endl;
            token = client_.disconnect(TIMEOUT, nullptr, *disconnListener_);
        };
        return common_try(fn, "Disconnect");
    }

    bool MqttClient::subscribe(mqtt::token_ptr &token, const std::string &topic, unsigned int qos)
    {
        std::function<void()> fn = [this, &token, &topic, &qos]() mutable
        {
            dinfo1("[MqttClient] Subscribing to '") << topic << "' with QOS=" << qos << "..." << std::endl;
            token = client_.subscribe(topic, qos, nullptr, *subListener_);
        };
        return common_try(fn, "Subscribe");
    }

    bool MqttClient::subscribe(const std::string &topic, unsigned int qos, bool wait, unsigned int wait_for)
    {
        mqtt::token_ptr token;
        bool res = subscribe(token, topic, qos);
        if (res && wait)
        {
            make_wait(token, wait_for);
        }
        return res;
    }

    bool MqttClient::publish(mqtt::token_ptr &token,
                             const std::string &topic,
                             const std::string &payload,
                             unsigned int qos)
    {
        std::function<void()> fn = [this, &token, &topic, &qos, &payload]() mutable
        {
            dinfo1("[MqttClient] Publishing to '") << topic << "': " << payload << std::endl;
            mqtt::message_ptr pubmsg = mqtt::make_message(topic, payload, qos, false);
            token = client_.publish(pubmsg, nullptr, *pubListener_);
        };
        return common_try(fn, "Publish");
    }

    bool MqttClient::publish(const std::string &topic,
                             const std::string &payload,
                             unsigned int qos,
                             bool wait,
                             unsigned int wait_for)
    {
        mqtt::token_ptr token;
        bool res = publish(token, topic, payload, qos);
        if (res && wait)
        {
            make_wait(token, wait_for);
        }
        return res;
    }

    bool MqttClient::connected() const
    {
        return client_.is_connected();
    }

    bool MqttClient::start_consume()
    {
        return this->consume_message(true);
    }

    bool MqttClient::stop_consume()
    {
        return this->consume_message(false);
    }

    bool MqttClient::consume_message(bool allow)
    {
        std::function<void()> fn = [this, &allow]() mutable
        {
            {
                lg lock(consumeGuard_);
                if (allow)
                {
                    client_.start_consuming();
                }
                else
                {
                    client_.stop_consuming();
                }
            }
            consumeFlag_.store(allow);
        };
        ostringstream oss;
        oss << "Turn " << (allow ? "on" : "off");
        return common_try(fn, oss.str().c_str());
    }

    bool MqttClient::pop_message(mqtt::binary &msg)
    {
        if (!consumeFlag_.load())
        {
            dinfo1("[MqttClient] Message consumption is disabled.\n").print();
            if (excPtr_)
            {
                excPtr_->push(false);
            }
            return false;
        }

        std::function<void()> fn = [this, &msg]() mutable
        {
            lg lock(consumeGuard_);
            mqtt::const_message_ptr msg_ptr;
            if (client_.try_consume_message(&msg_ptr))
            {
                msg = msg_ptr->to_string();
            }
        };
        return common_try(fn, "Pop message");
    }
    bool mqttcpp::MqttClient::is_consuming() const
    {
        return consumeFlag_.load();
    }
} // namespace mqttcpp
