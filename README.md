# MQTTClient

MQTTClient is a C++ project that provides an MQTT client using the Paho MQTT C++ library. This project includes an application that demonstrates how to connect to an MQTT broker, subscribe to a topic, and publish messages.

## Getting Started

### Prerequisites

- CMake 3.25 or higher
- A C++17 compatible compiler
- vcpkg for dependency management

### Building the Project

1. Clone the repository:

    ```sh
    git clone https://github.com/yourusername/MQTTClient.git
    cd MQTTClient
    ```

2. Install dependencies using vcpkg:

    ```sh
    vcpkg install paho-mqttpp3
    ```

3. Configure and build the project using CMake:

    ```sh
    cmake -B build -S .
    cmake --build build
    ```

### Testing the Project

This project supports unit testing using Google Test (GTest). The tests involve connecting to an MQTT broker, so a running MQTT broker (e.g., Mosquitto) is required for the tests to pass. If no broker is running, the tests will fail.

#### Configuration Options for Testing

The broker configuration (server address, client ID, and topic) can be provided in three ways, in the following priority order:

1. **Command-Line Arguments**:
    - `--server=<broker_address>`: Specify the MQTT broker address (e.g., `tcp://localhost:1883`).
    - `--client_id=<client_id>`: Specify the client ID.
    - `--topic=<topic>`: Specify the topic to subscribe to.

    Example:
    ```sh
    ./build/test/mqttclient.test --server=tcp://test.mosquitto.org:1883 --client_id=testClient --topic=test/topic
    ```

2. **Environment Variables**:
    - `MQTT_SERVER`: Specify the MQTT broker address.
    - `MQTT_CLIENT_ID`: Specify the client ID.
    - `MQTT_TOPIC`: Specify the topic to subscribe to.

    Example:
    ```sh
    export MQTT_SERVER="tcp://test.mosquitto.org:1883"
    export MQTT_CLIENT_ID="testClient"
    export MQTT_TOPIC="test/topic"
    ./build/test/mqttclient.test
    ```

3. **Default Values**:
    If neither command-line arguments nor environment variables are provided, the following default values will be used:
    - `SERVER_ADDRESS = "tcp://localhost:1883"`
    - `CLIENT_ID = "tesing_client_id"`
    - `TOPIC = "test"`

#### Steps to Run the Tests:

1. **Start an MQTT Broker**: Ensure that a Mosquitto MQTT broker is running locally or accessible over the network. You can start a Mosquitto broker locally using the following command:

    ```sh
    mosquitto
    ```

    By default, the broker will run on `localhost` and listen on port `1883`.

2. **Enable Testing**: Ensure the `ENABLE_TESTING` option is enabled in the `CMakePresets.json` file or pass it as a CMake argument:

    ```sh
    cmake -B build -S . -DENABLE_TESTING=ON
    cmake --build build
    ```

3. **Run the Tests**: Use the test executable with the desired configuration (command-line arguments, environment variables, or defaults).

    Example:
    ```sh
    ./build/test/mqttclient.test --server=tcp://test.mosquitto.org:1883 --client_id=testClient --topic=test/topic
    ```

### Project Options

The project provides several options that can be enabled or disabled in the `CMakeLists.txt` file:

- **BUILD_SHARED_LIBS**: Build shared libraries (default: **ON**)
- **ENABLE_CMAKE_FORMAT**: Enable CMake Format (default: **ON**)
- **ENABLE_CLANG_FORMAT**: Enable Clang Format (default: **ON**)
- **ENABLE_TESTING**: Enable Testing (default: **OFF**)
- **ENABLE_ADDRESS_SANITIZER**: Enable Address Sanitizer (default: **ON**)
- **ENABLE_UNDEFINED_SANITIZER**: Enable Undefined Sanitizer (default: **OFF**)
- **ENABLE_LEAK_SANITIZER**: Enable Leak Sanitizer (default: **OFF**)
- **ENABLE_THREAD_SANITIZER**: Enable Thread Sanitizer (default: **OFF**)
- **ENABLE_LTO**: Enable Link Time Optimization (default: **OFF**)

## Version Management

This project uses semantic versioning (MAJOR.MINOR.PATCH) and automated version management through GitHub Actions.

### Version Bump Rules

- **MAJOR** version (X.0.0): Breaking changes
- **MINOR** version (0.X.0): New features
- **PATCH** version (0.0.X): Bug fixes

### How Version Updates Work

1. **PR Labels**:
   - Add appropriate version label to your PR:
     - `major`: For breaking changes
     - `minor`: For new features
     - `patch`: For bug fixes

2. **Automated Updates**:
   - When PR is merged, version is automatically bumped
   - New version is calculated based on PR label
   - CMakeLists.txt is updated
   - New release is created

3. **Release Notes**:
   - PR description is used for release notes
   - Version change information is added
   - Release is tagged with new version

## Contributing

We welcome contributions to improve MQTTClient! Here are some ways you can contribute:

- **Bug Reports**: Found a bug? Open an issue and provide as much detail as possible.
- **Feature Requests**: Have an idea for a new feature? Let us know by opening an issue.
- **Code Contributions**: Submit a pull request with your improvements or fixes. Make sure to follow the project's coding style and include tests for your changes.
- **Documentation**: Help us improve the documentation by fixing typos, adding examples, or clarifying instructions.

### How to Contribute

1. Fork the repository and create a new branch for your changes.
2. Make your changes and commit them with clear and concise messages.
3. Push your branch to your fork and submit a pull request.
4. Add appropriate version label to your PR:
   - `major`: For breaking changes
   - `minor`: For new features
   - `patch`: For bug fixes

### Areas to Contribute

- **Testing**: Add more test cases to improve coverage.
- **Features**: Implement additional MQTT features like QoS handling, retained messages, or advanced connection options.
- **Performance**: Optimize the client for better performance in high-load scenarios.
- **Documentation**: Add examples, tutorials, or improve the existing documentation.

## GitHub Workflows

This project uses GitHub Actions for automation. See [WORKFLOWS.md](WORKFLOWS.md) for detailed information about:

- CI/CD pipeline
- Version management
- Automated testing
- Release process

## License

This project is licensed under the MIT License - see the LICENSE file for details.