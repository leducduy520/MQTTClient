#include "mqttclient.hpp"
#include <sstream>
#include <memory>

const unsigned int QOS = 1;
const auto TIMEOUT = std::chrono::seconds(5);
const std::string SERVER_ADDRESS{"tcp://localhost:30520"};
const std::string CLIENT_ID{"duyld520"};
const std::string TOPIC{"hello"};

bool subscribe_success = false;
bool publish_success = false;
bool connected = false;
std::condition_variable cv;
std::mutex event_mutex;

void main_event_handle(mqttcpp::CallbackEvent event, mqttcpp::CallbackVariant info)
{
    using namespace mqttcpp;
    auto client = MqttClient::Instance.get();
    switch (event)
    {
    case CallbackEvent::EVENT_CONNECTED:
    {
        std::cout << "Connected to broker" << std::endl;
        {
            auto cause = info.asString();
            std::cout << cause << "\n";
            std::lock_guard lock(event_mutex);
            cv.notify_one();
        }
    }
    break;
    case CallbackEvent::EVENT_DISCONNECTED:
    {
        std::cout << "Disconnected from broker" << std::endl;
        {
            std::lock_guard lock(event_mutex);
            cv.notify_one();
        }
    }
    break;
    case CallbackEvent::EVENT_ACTION_SUCCESS:
    {
        mqtt::token_ptr ptok = info.asToken();
        if (ptok)
        {
            std::ostringstream oss;
            oss << "Action " << ptok->get_type() << " success\n";
            if (ptok->get_type() == mqtt::token::SUBSCRIBE)
            {
                oss << "Subscribed to topic: " << ptok->get_topics()->c_arr()[0] << std::endl;
                {
                    std::lock_guard lock(event_mutex);
                    subscribe_success = true;
                    cv.notify_one();
                }
            }
            else if (ptok->get_type() == mqtt::token::PUBLISH)
            {
                oss << "Published to topic: " << ptok->get_topics()->c_arr()[0]
                    << " with message id: " << ptok->get_message_id() << std::endl;
                {
                    std::lock_guard lock(event_mutex);
                    publish_success = true;
                    cv.notify_one();
                }
            }
            std::cout << oss.str();
        }
    }
    break;
    case CallbackEvent::EVENT_CONNECTION_UPDATE:
    {
        auto* data = &info.asConnectData();
    }
    default:
        break;
    }
}

int main()
{
    using namespace mqttcpp;
    auto client = mqttcpp::MqttClient::Instance.get();
    auto connOpts = mqtt::connect_options_builder().automatic_reconnect().finalize();
    client = new mqttcpp::MqttClient(SERVER_ADDRESS, CLIENT_ID, connOpts);
    client->set_event_handler(main_event_handle);
    if (!client->connect(true, 5))
    {
        return 0;
    }
    {
        std::unique_lock lock(event_mutex);
        cv.wait(lock, [&] { return client->connected(); });
    }
    std::cout << "Press 'Enter' to continue\n";
    (void)getchar();
    {
        std::unique_lock lock(event_mutex);
        client->subscribe(TOPIC, 1, true, 5);
        cv.wait(lock, [&] { return subscribe_success; });
    }
    (void)getchar();
    std::cout << "Press 'Enter' to continue\n";
    {
        std::unique_lock lock(event_mutex);
        client->publish(TOPIC, "Hello broker", 1, true, 5);
        cv.wait(lock, [&] { return publish_success; });
    }
    std::cout << "Press 'Enter' to continue\n";
    (void)getchar();
    {
        client->disconnect(true, 5);
        std::unique_lock lock(event_mutex);
        cv.wait(lock, [&] { return !client->connected(); });
    }
    return 0;
}
