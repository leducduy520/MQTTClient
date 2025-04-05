#include "mqttclient.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <string>

using namespace mqtt;
using namespace mqttcpp;

// Default values for broker configuration
std::string SERVER_ADDRESS = "tcp://localhost:1883";
std::string CLIENT_ID = "tesing_client_id";
std::string TOPIC = "test";

// Helper function to get environment variable or default value
std::string getEnvOrDefault(const char* envVar, const std::string& defaultValue)
{
    const char* value = std::getenv(envVar);
    return value ? std::string(value) : defaultValue;
}

// Test fixture
class MqttClientTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create the client with test server and client ID
        auto connOpts = mqtt::connect_options_builder().automatic_reconnect().finalize();
        client = std::make_unique<MqttClient>(SERVER_ADDRESS, CLIENT_ID, connOpts);
    }

    void TearDown() override
    {
        client.reset();
    }

    std::unique_ptr<MqttClient> client;
};

// Test that verifies successful connection with default options
TEST_F(MqttClientTest, ShouldConnectToBrokerWithDefaultOptions)
{
    // Arrange
    mqtt::token_ptr token;

    // Act
    bool result = client->connect(token);
    EXPECT_TRUE(result);
    token->wait();

    // Assert
    EXPECT_TRUE(token != nullptr);

    // Verify connected state
    EXPECT_TRUE(client->connected());

    // Disconnecting should work too
    EXPECT_TRUE(client->disconnect());
}

// Test that verifies subscription and unsubscription to a topic
TEST_F(MqttClientTest, ShouldSubscribeToTopicWithSpecifiedQoSAndWait)
{
    // Arrange
    const std::string testTopic = "test/topic";
    const unsigned int testQos = 1;
    mqtt::token_ptr token;

    // Connect to the broker
    bool result = client->connect(token);
    EXPECT_TRUE(result);
    token->wait();

    // Act: Subscribe to the topic
    result = client->subscribe(token, testTopic, testQos);
    EXPECT_TRUE(result);
    token->wait();

    // Assert: Verify subscription was successful
    EXPECT_TRUE(result);
    EXPECT_TRUE(token != nullptr);

    // Test the overloaded version with wait parameter
    bool subscribeWithWaitResult = client->subscribe(testTopic, testQos, true, 1000);
    EXPECT_TRUE(subscribeWithWaitResult);

    // Act: Unsubscribe from the topic
    result = client->unsubscribe(testTopic, true);
    EXPECT_TRUE(result);

    // Assert: Verify unsubscription was successful
    EXPECT_TRUE(client->connected());

    // Disconnect from the broker
    EXPECT_TRUE(client->disconnect());
}

// Main function to parse command-line arguments and run tests
int main(int argc, char** argv)
{
    // Default values for broker configuration
    std::string server_address, client_id, topic;

    // Parse command-line arguments for broker configuration
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg.find("--server=") == 0)
        {
            server_address = arg.substr(9);
        }
        else if (arg.find("--client_id=") == 0)
        {
            client_id = arg.substr(12);
        }
        else if (arg.find("--topic=") == 0)
        {
            topic = arg.substr(8);
        }
    }

    // If not provided, use environment variables or default values
    if (server_address.empty())
    {
        server_address = getEnvOrDefault("MQTT_SERVER", SERVER_ADDRESS);
    }
    if (client_id.empty())
    {
        client_id = getEnvOrDefault("MQTT_CLIENT_ID", CLIENT_ID);
    }
    if (topic.empty())
    {
        topic = getEnvOrDefault("MQTT_TOPIC", TOPIC);
    }

    // Set the server address, client ID, and topic for the tests
    SERVER_ADDRESS = server_address;
    CLIENT_ID = client_id;
    TOPIC = topic;

    // Initialize Google Test framework
    ::testing::InitGoogleTest(&argc, argv);

    // Run all tests
    return RUN_ALL_TESTS();
}
