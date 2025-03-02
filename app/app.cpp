#include "mqttclient.hpp"
#include <sstream>

bool subscribe_success = false;
bool connected = false;
std::condition_variable cv;
std::mutex event_mutex;

void main_event_handle(mqttcpp::CallbackEvent event,
                       mqttcpp::CallbackVariant info) {
  using namespace mqttcpp;
  auto client = MqttClient::Instance.get();
  switch (event) {
  case CallbackEvent::EVENT_CONNECTED: {
    std::cout << "Connected to broker" << std::endl;
    {
      std::lock_guard lock(event_mutex);
      cv.notify_one();
    }
  } break;
  case CallbackEvent::EVENT_DISCONNECTED: {
    std::cout << "Disconnected from broker" << std::endl;
    {
      std::lock_guard lock(event_mutex);
      cv.notify_one();
    }
  } break;
  case CallbackEvent::EVENT_ACTION_SUCCESS: {
    mqtt::token_ptr ptok = info.asToken();
    if (ptok) {
      std::ostringstream oss;
      oss << "Action " << ptok->get_type() << " success\n";
      if (ptok->get_type() == mqtt::token::SUBSCRIBE) {
        oss << "Subscribed to topic: " << ptok->get_topics() << std::endl;
        {
          std::lock_guard lock(event_mutex);
          subscribe_success = true;
          cv.notify_one();
        }
      }
      std::cout << oss.str();
    }
  }
  default:
    break;
  }
}

int main() {
  using namespace mqttcpp;
  auto client = mqttcpp::MqttClient::Instance.get();
  client = new mqttcpp::MqttClient(SERVER_ADDRESS, CLIENT_ID);
  client->set_event_handler(main_event_handle);
  if (!client->connect(true, 5)) {
    return 0;
  }
  {
    std::unique_lock lock(event_mutex);
    cv.wait(lock, [&] { return client->connected(); });
  }
  {
    std::unique_lock lock(event_mutex);
    client->subscribe(TOPIC, 1, true, 5);
    cv.wait(lock, [&] { return subscribe_success; });
  }
  {
    client->disconnect(true, 5);
    std::unique_lock lock(event_mutex);
    cv.wait(lock, [&] { return !client->connected(); });
  }
  return 0;
}
