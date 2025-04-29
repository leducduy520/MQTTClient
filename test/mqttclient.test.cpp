#include "mqttclient.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <chrono>
#include <thread>
#include <future>
#include <string>
#include <cstdlib>

using namespace mqtt;
using namespace mqttcpp;

const std::string SERVER_ADDRESS{std::getenv("MQTT_SERVER") ? std::getenv("MQTT_SERVER")
                                                            : "tcp://localhost:1883"};
const std::string CLIENT_ID{std::getenv("MQTT_CLIENT_ID") ? std::getenv("MQTT_CLIENT_ID")
                                                            : "test_client"};
const std::string TOPIC{std::getenv("MQTT_TOPIC") ? std::getenv("MQTT_TOPIC")
                                                  : "test/topic"};
const int QOS = std::stoi(std::getenv("MQTT_QOS") ? std::getenv("MQTT_QOS")
                                                  : "1");
const int TIMEOUT_MS = 4000;

// Test fixture
class MqttClientTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create the client with test server and client ID
        auto connOpts = mqtt::connect_options_builder()
                            .automatic_reconnect()
                            .keep_alive_interval(std::chrono::seconds(30))
                            .clean_session(true)
                            .finalize();
        client = std::make_unique<MqttClient>(SERVER_ADDRESS, CLIENT_ID, connOpts);
    }

    void TearDown() override
    {
        if (client && client->connected())
        {
            client->disconnect(true);
        }
        client.reset();
    }

    std::unique_ptr<MqttClient> client;
};

// Connection Tests
TEST_F(MqttClientTest, ShouldConnectToBrokerWithDefaultOptions)
{
    // Arrange
    mqtt::token_ptr token;

    // Act
    bool result = client->connect(token);
    token->wait();

    // Assert
    EXPECT_TRUE(result);
    EXPECT_TRUE(token != nullptr);
    EXPECT_TRUE(client->connected());
}

// Event Handler Tests
TEST_F(MqttClientTest, ShouldHandleConnectionEvents)
{
    // Arrange
    std::promise<CallbackEvent> eventPromise;
    auto eventFuture = eventPromise.get_future();
    bool eventReceived = false;

    client->set_event_handler([&eventPromise, &eventReceived](CallbackEvent event, CallbackVariant info) {
        if (event == CallbackEvent::EVENT_CONNECTED)
        {
            eventReceived = true;
            eventPromise.set_value(event);
        }
    });

    // Act
    mqtt::token_ptr token;
    bool result = client->connect(token);
    token->wait();

    // Assert
    EXPECT_TRUE(result);
    EXPECT_TRUE(client->connected());
    auto status = eventFuture.wait_for(std::chrono::milliseconds(TIMEOUT_MS));
    EXPECT_EQ(status, std::future_status::ready);
    EXPECT_TRUE(eventReceived);
}

// Subscription Tests
TEST_F(MqttClientTest, ShouldSubscribeToTopicWithSpecifiedQoS)
{
    // Arrange
    mqtt::token_ptr token;
    ASSERT_TRUE(client->connect(token));
    token->wait();

    // Act
    bool result = client->subscribe(token, TOPIC, QOS);
    token->wait();

    // Assert
    EXPECT_TRUE(result);
    EXPECT_TRUE(token != nullptr);
}

TEST_F(MqttClientTest, ShouldSubscribeToTopicWithWait)
{
    // Arrange
    mqtt::token_ptr token;
    ASSERT_TRUE(client->connect(token));
    token->wait();

    // Act
    bool result = client->subscribe(TOPIC, QOS, true, TIMEOUT_MS);

    // Assert
    EXPECT_TRUE(result);
}

TEST_F(MqttClientTest, ShouldUnsubscribeFromTopic)
{
    // Arrange
    mqtt::token_ptr token;
    ASSERT_TRUE(client->connect(token));
    token->wait();
    ASSERT_TRUE(client->subscribe(token, TOPIC, QOS));
    token->wait();

    // Act
    bool result = client->unsubscribe(TOPIC, true);

    // Assert
    EXPECT_TRUE(result);
}

// Publishing Tests
TEST_F(MqttClientTest, ShouldPublishMessageToTopic)
{
    // Arrange
    mqtt::token_ptr token;
    ASSERT_TRUE(client->connect(token));
    token->wait();
    const std::string payload = "test message";

    // Act
    bool result = client->publish(token, TOPIC, payload, QOS);
    token->wait();

    // Assert: Verify subscription was successful
    EXPECT_TRUE(result);
    EXPECT_TRUE(token != nullptr);
}

TEST_F(MqttClientTest, ShouldPublishMessageWithWait)
{
    // Arrange
    mqtt::token_ptr token;
    ASSERT_TRUE(client->connect(token));
    token->wait();
    const std::string payload = "test message";

    // Act
    bool result = client->publish(TOPIC, payload, QOS, true, TIMEOUT_MS);

    // Assert
    EXPECT_TRUE(result);
}

// Message Reception Tests
TEST_F(MqttClientTest, ShouldReceivePublishedMessage)
{
    // Arrange
    mqtt::token_ptr token;
    ASSERT_TRUE(client->connect(token));
    token->wait();
    ASSERT_TRUE(client->subscribe(token, TOPIC, QOS));
    token->wait();

    const std::string payload = "test message";
    std::promise<std::string> messagePromise;
    auto messageFuture = messagePromise.get_future();

    // Set up message callback
    client->set_event_handler([&messagePromise](CallbackEvent event, CallbackVariant info) {
        if (event == CallbackEvent::EVENT_MESSAGE_ARRIVED)
        {
            auto msg = info.asMessage();
            if (msg)
            {
                messagePromise.set_value(msg->get_payload_str());
            }
        }
    });

    // Act
    ASSERT_TRUE(client->publish(TOPIC, payload, QOS, true));

    // Wait for message with timeout
    auto status = messageFuture.wait_for(std::chrono::milliseconds(TIMEOUT_MS));

    // Assert
    EXPECT_EQ(status, std::future_status::ready);
    EXPECT_EQ(messageFuture.get(), payload);
}

// Connection State Tests
TEST_F(MqttClientTest, ShouldHandleDisconnection)
{
    // Arrange
    mqtt::token_ptr token;
    ASSERT_TRUE(client->connect(token));
    token->wait();

    // Act
    bool result = client->disconnect(true);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_FALSE(client->connected());
}

TEST_F(MqttClientTest, ShouldHandleReconnection)
{
    // Arrange
    mqtt::token_ptr token;
    ASSERT_TRUE(client->connect(token));
    token->wait();
    ASSERT_TRUE(client->disconnect(true));

    // Act
    bool result = client->connect(token);
    token->wait();

    // Assert
    EXPECT_TRUE(result);
    EXPECT_TRUE(client->connected());
}

TEST_F(MqttClientTest, ShouldHandleInvalidQoS)
{
    // Arrange
    mqtt::token_ptr token;
    ASSERT_TRUE(client->connect(token));
    token->wait();
    const int invalidQoS = 3; // QoS can only be 0, 1, or 2

    // Act
    bool result = client->subscribe(token, TOPIC, invalidQoS);
    token->wait();

    // Assert
    EXPECT_FALSE(result);
}

// Message Consumption Tests
TEST_F(MqttClientTest, ShouldHandleMessageConsumption)
{
    // Arrange
    mqtt::token_ptr token;
    ASSERT_TRUE(client->connect(token));
    token->wait();
    ASSERT_TRUE(client->subscribe(token, TOPIC, QOS));
    token->wait();

    // Act
    bool result = client->start_saving_message();
    ASSERT_TRUE(result);
    ASSERT_TRUE(client->is_saving_message());

    // Publish a message
    const std::string payload = "test message";
    ASSERT_TRUE(client->publish(TOPIC, payload, QOS, true));

    // Try to consume the message
    mqtt::binary msg;
    result = client->get_next_message(msg);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_TRUE(client->stop_saving_message());
    EXPECT_FALSE(client->is_saving_message());
}
