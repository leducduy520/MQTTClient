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

### Running the Application

After building the project, you can run the application:

```sh
./build/app/app
```

### Project Options

The project provides several options that can be enabled or disabled in the CMakeLists.txt file:

- BUILD_SHARED_LIBS: Build shared libraries (default: ON)
- ENABLE_CMAKE_FORMAT: Enable CMake Format (default: ON)
- ENABLE_CLANG_FORMAT: Enable Clang Format (default: ON)
- ENABLE_TESTING: Enable Testing (default: OFF)
- ENABLE_ADDRESS_SANITIZER: Enable Address Sanitizer (default: ON)
- ENABLE_UNDEFINED_SANITIZER: Enable Undefined Sanitizer (default: OFF)
- ENABLE_LEAK_SANITIZER: Enable Leak Sanitizer (default: OFF)
- ENABLE_THREAD_SANITIZER: Enable Thread Sanitizer (default: OFF)
- ENABLE_LTO: Enable Link Time Optimization (default: OFF)

## License

This project is licensed under the MIT License - see the LICENSE file for details.