
#include "mqttclient.hpp"
#include <gtest/gtest.h>
#include <memory>

using namespace mqtt;
using namespace mqttcpp;

const std::string SERVER_ADDRESS{"tcp://localhost:30520"};
const std::string CLIENT_ID{"duyld520"};
const std::string TOPIC{"hello"};

// Test fixture
class MqttClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create the client with test server and client ID
        auto connOpts =
        mqtt::connect_options_builder().user_name("duyle").password("552200").automatic_reconnect().finalize();
        client = std::make_unique<MqttClient>(SERVER_ADDRESS, CLIENT_ID, connOpts);
    }

    void TearDown() override {
        client.reset();
    }

    std::unique_ptr<MqttClient> client;
};

// Test that verifies successful connection with default options
TEST_F(MqttClientTest, ShouldConnectToBrokerWithDefaultOptions) {
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
    EXPECT_TRUE(client->disconnect(false));
}

TEST_F(MqttClientTest, ShouldSubscribeToTopicWithSpecifiedQoSAndWait) {
    // Arrange
    const std::string testTopic = "test/topic";
    const unsigned int testQos = 1;
    mqtt::token_ptr token;

    bool result = client->connect(token);
    EXPECT_TRUE(result);
    token->wait();

    // Act
    result = client->subscribe(token, testTopic, testQos);
    EXPECT_TRUE(result);
    token->wait();

    // Assert
    EXPECT_TRUE(result);
    EXPECT_TRUE(token != nullptr);

    // Test the overloaded version with wait parameter
    bool subscribeWithWaitResult = client->subscribe(testTopic, testQos, true, 1000);
    EXPECT_TRUE(subscribeWithWaitResult);

    // Verify unsubscribe works too
    EXPECT_TRUE(client->unsubscribe(testTopic, true));
}
