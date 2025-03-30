# Test CMakeLists.txt

# Find GTest package
find_package(GTest REQUIRED)

# Create test executable
add_executable(mqttclient_tests
    mqttclient.test.cpp
)

# Link against the necessary libraries
target_link_libraries(mqttclient_tests
    PRIVATE
    MQTTClient
    GTest::GTest
    GTest::Main
)

# Include directories
target_include_directories(mqttclient_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/mqttclient
)

# Add the test to CTest
include(GoogleTest)
gtest_discover_tests(mqttclient_tests)

# Set C++ standard
set_target_properties(mqttclient_tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Apply compiler warnings
if(MSVC)
    target_compile_options(mqttclient_tests PRIVATE /W4)
else()
    target_compile_options(mqttclient_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Apply sanitizers if enabled
if(ENABLE_ADDRESS_SANITIZER)
    target_compile_options(mqttclient_tests PRIVATE -fsanitize=address)
    target_link_options(mqttclient_tests PRIVATE -fsanitize=address)
endif()

if(ENABLE_UNDEFINED_SANITIZER)
    target_compile_options(mqttclient_tests PRIVATE -fsanitize=undefined)
    target_link_options(mqttclient_tests PRIVATE -fsanitize=undefined)
endif()

if(ENABLE_LEAK_SANITIZER)
    target_compile_options(mqttclient_tests PRIVATE -fsanitize=leak)
    target_link_options(mqttclient_tests PRIVATE -fsanitize=leak)
endif()

if(ENABLE_THREAD_SANITIZER)
    target_compile_options(mqttclient_tests PRIVATE -fsanitize=thread)
    target_link_options(mqttclient_tests PRIVATE -fsanitize=thread)
endif()