#include "mqttclient.hpp"
#include "monitor.hpp"
#include <sstream>

using namespace mqtt;

namespace mqttcpp
{
    std::unique_ptr<MqttClient> MqttClient::Instance{nullptr};

    static std::string mqttEventToString(CallbackEvent event)
    {
        switch (event)
        {
        case CallbackEvent::EVENT_CONNECTED:
            return "EVENT_CONNECTED";
        case CallbackEvent::EVENT_DISCONNECTED:
            return "EVENT_DISCONNECTED";
        case CallbackEvent::EVENT_CONNECTION_UPDATE:
            return "EVENT_CONNECTION_UPDATE";
        case CallbackEvent::EVENT_CONNECTION_LOST:
            return "EVENT_CONNECTION_LOST";
        case CallbackEvent::EVENT_MESSAGE_ARRIVED:
            return "EVENT_MESSAGE_ARRIVED";
        case CallbackEvent::EVENT_DELIVERY_COMPLETE:
            return "EVENT_DELIVERY_COMPLETE";
        case CallbackEvent::EVENT_ACTION_SUCCESS:
            return "EVENT_ACTION_SUCCESS";
        case CallbackEvent::EVENT_ACTION_FAILURE:
            return "EVENT_ACTION_FAILURE";
        default:
            return "UNKNOWN";
        }
    }

    DefaultActionListener::DefaultActionListener(MqttClient* parent) : parent_(parent)
    {}

    void DefaultActionListener::on_failure(const mqtt::token& tok)
    {
        if (parent_)
        {
            parent_->self_handle_callback_event(CallbackEvent::EVENT_ACTION_FAILURE,
                                                mqtt::token::create(tok.get_type(),
                                                                    *tok.get_client(),
                                                                    tok.get_topics(),
                                                                    tok.get_user_context(),
                                                                    *tok.get_action_callback()));
        }
    }

    void DefaultActionListener::on_success(const mqtt::token& tok)
    {
        if (parent_)
        {
            parent_->self_handle_callback_event(CallbackEvent::EVENT_ACTION_SUCCESS,
                                                token::create(tok.get_type(),
                                                              *tok.get_client(),
                                                              tok.get_topics(),
                                                              tok.get_user_context(),
                                                              *tok.get_action_callback()));
        }
    }

    bool MqttClient::common_try(std::function<void()> fn, const char* fnId)
    {
        try
        {
            fn();
            if (excPtr_)
            {
                *excPtr_ = ExceptionTrace();
            }
            return true;
        }
        catch (const mqtt::exception& exc)
        {
            derror1("[MqttClient] %s error: ", fnId) << exc.what() << std::endl;
            if (excPtr_)
            {
                *excPtr_ = ExceptionTrace(exc);
            }
        }
        catch (const std::exception& exc)
        {
            derror1("[MqttClient] %s error: Standard exception\n", fnId).print();
            if (excPtr_)
            {
                *excPtr_ = ExceptionTrace(exc);
            }
        }
        catch (...)
        {
            derror1("[MqttClient] %s error: Unknown exception\n", fnId).print();
            if (excPtr_)
            {
                char buffer[100];
                sprintf(buffer, "Unknown exception from excuting \"%s\"\n", fnId);
                *excPtr_ = ExceptionTrace(buffer);
            }
        }
        return false;
    }

    void MqttClient::self_handle_callback_event(CallbackEvent event, CallbackVariant info)
    {
        dinfo2("[MqttClient] Event ") << mqttEventToString(event) << std::endl;
        switch (event)
        {
        case CallbackEvent::EVENT_CONNECTED:
        {
            std::ostringstream oss;
            oss << "Connected to broker." << std::endl;
            if (!info.asString().empty())
            {
                oss << " Cause: " << info.asString();
            }
            dinfo1(oss.str().c_str()) << std::endl;
        }
        break;
        case CallbackEvent::EVENT_DISCONNECTED:
        {
            std::ostringstream oss;
            oss << "Disconnected from broker.";
            disconnect_data disconnData = info.asDisconnectData();
            if (!disconnData.props.empty())
            {
                oss << " ReasonString: " << get<string>(disconnData.props, property::REASON_STRING)
                    << ", ReasonCode: " << disconnData.reason;
            }
            dinfo1(oss.str().c_str()) << std::endl;
        }
        break;
        case CallbackEvent::EVENT_CONNECTION_UPDATE:
        {
            auto* data = &info.asConnectData();
            dinfo1("Connection update received with:\n")
                << "  - current username: " << data->get_user_name()
                << "\n  - current password: " << data->get_password() << std::endl;
        }
        break;
        case CallbackEvent::EVENT_CONNECTION_LOST:
        {
            std::ostringstream oss;
            oss << "Connection lost.";
            if (!info.asString().empty())
            {
                oss << " Cause: " << info.asString();
            }
            dinfo1(oss.str().c_str()) << std::endl;
        }
        break;
        case CallbackEvent::EVENT_MESSAGE_ARRIVED:
        {
            mqtt::const_message_ptr msg = info.asMessage();
            if (msg)
            {
                std::ostringstream oss;
                oss << "Topic: " << msg->get_topic() << ", Payload: " << msg->to_string()
                    << ", Retained: " << (msg->is_retained() ? "true" : "false");
                dinfo1("Message arrived: ") << oss.str() << std::endl;
            }
        }
        break;
        case CallbackEvent::EVENT_ACTION_SUCCESS:
        {
            mqtt::token_ptr ptok = info.asToken();
            if (ptok)
            {
                std::ostringstream oss;
                dinfo1("") << "Action " << ptok->get_type() << " success\n" << ddbg::end();

                if (ptok->get_type() == mqtt::token::DISCONNECT)
                {
                    mqtt::properties props;
                    props.add({mqtt::property::REASON_STRING, "User has manually disconnected to brocker"});
                    this->self_handle_callback_event(CallbackEvent::EVENT_DISCONNECTED,
                                                     disconnect_data{props, mqtt::ReasonCode::NORMAL_DISCONNECTION});
                }
            }
        }
        break;
        case CallbackEvent::EVENT_ACTION_FAILURE:
        {
            mqtt::token_ptr ptok = info.asToken();
            if (ptok)
            {
                std::ostringstream oss;
                oss << "Action " << ptok->get_type() << " fail\n";
                dinfo1(oss.str().c_str()).print();
            }
        }
        break;
        default:
            break;
        }

        if (exteventHandler_)
        {
            ddebug1("Send information to external event handler\n").print();
            exteventHandler_(event, info);
        }
    }

    MqttClient::MqttClient(const std::string& serverAddress, const std::string& clientId)
        : pubListener_(new DefaultActionListener(this)), subListener_(new DefaultActionListener(this)),
          unsubListener_(new DefaultActionListener(this)), connListener_(new DefaultActionListener(this)),
          disconnListener_(new DefaultActionListener(this)), consumeFlag_(false), client_(serverAddress, clientId),
          excPtr_(new ExceptionTrace())
    {
        connOpts_.set_keep_alive_interval(60);
        connOpts_.set_clean_session(true);
        connOpts_.set_automatic_reconnect(true);
        connOpts_.set_connect_timeout(10);

        set_default_handler();
    }

    MqttClient::MqttClient(const std::string& serverAddress,
                           const std::string& clientId,
                           mqtt::connect_options connectOptions)
        : connOpts_(connectOptions), pubListener_(new DefaultActionListener(this)),
          subListener_(new DefaultActionListener(this)), unsubListener_(new DefaultActionListener(this)),
          connListener_(new DefaultActionListener(this)), disconnListener_(new DefaultActionListener(this)),
          consumeFlag_(false), client_(serverAddress, clientId), excPtr_(new ExceptionTrace())
    {
        set_default_handler();
    }

    MqttClient::MqttClient(const std::string& serverAddress,
                           const std::string& clientId,
                           mqtt::create_options createOptions,
                           mqtt::connect_options connectOptions)
        : connOpts_(connectOptions), pubListener_(new DefaultActionListener(this)),
          subListener_(new DefaultActionListener(this)), unsubListener_(new DefaultActionListener(this)),
          connListener_(new DefaultActionListener(this)), disconnListener_(new DefaultActionListener(this)),
          consumeFlag_(false), client_(serverAddress, clientId, createOptions, nullptr), excPtr_(new ExceptionTrace())
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

    void MqttClient::set_event_handler(std::function<void(CallbackEvent, CallbackVariant)> handler)
    {
        exteventHandler_ = handler;
    }

    bool MqttClient::connect(mqtt::token_ptr& token)
    {
        std::function<void()> fn = [this, &token]() mutable {
            dinfo1("[MqttClient] Connecting to broker...\n").print();
            token = client_.connect(connOpts_, nullptr, *connListener_);
        };
        return common_try(fn, "Connect");
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

    bool MqttClient::disconnect(mqtt::token_ptr& token)
    {
        std::function<void()> fn = [this, &token]() mutable {
            dinfo1("[MqttClient] Disconnecting...") << std::endl;
            token = client_.disconnect(TIMEOUT, nullptr, *disconnListener_);
        };
        return common_try(fn, "Disconnect");
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

    bool MqttClient::subscribe(mqtt::token_ptr& token, const std::string& topic, unsigned int qos)
    {
        std::function<void()> fn = [this, &token, &topic, &qos]() mutable {
            dinfo1("[MqttClient] Subscribing to '") << topic << "' with QOS=" << qos << "..." << std::endl;
            token = client_.subscribe(topic,
                                      qos,
                                      nullptr,
                                      *subListener_,
                                      mqtt::subscribe_options(true, true, subscribe_options::DONT_SEND_RETAINED));
        };
        return common_try(fn, "Subscribe");
    }

    bool MqttClient::subscribe(const std::string& topic, unsigned int qos, bool wait, unsigned int wait_for)
    {
        mqtt::token_ptr token;
        bool res = subscribe(token, topic, qos);
        if (res && wait)
        {
            dinfo2("Gone here\n");
            make_wait(token, wait_for);
        }
        return res;
    }

    bool MqttClient::unsubscribe(mqtt::token_ptr& token, const std::string& topic)
    {
        std::function<void()> fn = [this, &token, &topic]() mutable {
            dinfo1("[MqttClient] Unsubscribing from '") << topic << "'..." << std::endl;
            token = client_.unsubscribe(topic, nullptr, *unsubListener_);
        };
        return common_try(fn, "Unsubscribe");
    }

    bool MqttClient::unsubscribe(const std::string& topic, bool wait, unsigned int wait_for)
    {
        mqtt::token_ptr token;
        bool res = unsubscribe(token, topic);
        if (res && wait)
        {
            make_wait(token, wait_for);
        }
        return res;
    }

    bool MqttClient::publish(mqtt::token_ptr& token,
                             const std::string& topic,
                             const std::string& payload,
                             unsigned int qos)
    {
        std::function<void()> fn = [this, &token, &topic, &qos, &payload]() mutable {
            dinfo1("[MqttClient] Publishing to '") << topic << "': " << payload << std::endl;
            mqtt::message_ptr pubmsg = mqtt::make_message(topic, payload, qos, false);
            token = client_.publish(pubmsg, nullptr, *pubListener_);
        };
        return common_try(fn, "Publish");
    }

    bool MqttClient::publish(const std::string& topic,
                             const std::string& payload,
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

    bool MqttClient::consume_message(bool allow)
    {
        std::function<void()> fn = [this, &allow]() mutable {
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

    bool MqttClient::get_next_message(mqtt::binary& msg)
    {
        if (!consumeFlag_.load())
        {
            dinfo1("[MqttClient] Message consumption is disabled.\n").print();
            if (excPtr_)
            {
                *excPtr_ = ExceptionTrace();
            }
            return false;
        }

        std::function<void()> fn = [this, &msg]() mutable {
            lg lock(consumeGuard_);
            mqtt::const_message_ptr msg_ptr;
            if (client_.try_consume_message(&msg_ptr))
            {
                msg = msg_ptr->to_string();
            }
        };
        return common_try(fn, "Pop message");
    }

} // namespace mqttcpp
