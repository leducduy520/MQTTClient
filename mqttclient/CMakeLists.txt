# Find required dependencies
find_package(PahoMqttCpp CONFIG REQUIRED)

# Define library target
add_library(MQTTClient STATIC "mqttclient.cpp" "mqttclient.hpp" "monitor.hpp")

# Link dependencies
target_link_libraries(
    MQTTClient PUBLIC $<IF:$<STREQUAL:$<PLATFORM_ID>,Linux>, PahoMqttCpp::paho-mqttpp3-static,
                      PahoMqttCpp::paho-mqttpp3>
    )

# Configure include directories
target_include_directories(
    MQTTClient PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
    )

# Set target properties
set_target_properties(MQTTClient PROPERTIES FOLDER "PingCCU Service")

# Configure warnings
target_set_warnings(TARGET MQTTClient ENABLE TRUE AS_ERRORS FALSE)

# Add precompiled header
target_precompile_headers(
    MQTTClient
    PRIVATE
    # Standard library headers
    <string>
    <vector>
    <memory>
    <functional>
    <mutex>
    <condition_variable>
    <atomic>
    <stdexcept>
    <iostream>
    # MQTT library headers
    <mqtt/async_client.h>
    <mqtt/token.h>
    <mqtt/connect_options.h>
    <mqtt/message.h>
    <mqtt/callback.h>
    )

# Install targets
install(
    TARGETS MQTTClient
    EXPORT MQTTClientTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

# Install header files
install(FILES "mqttclient.hpp" "monitor.hpp"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
        )

# Export targets
install(
    EXPORT MQTTClientTargets
    FILE MQTTClientTargets.cmake
    NAMESPACE MQTTClient::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MQTTClient
    )

# Generate package config files
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/MQTTClientConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/MQTTClientConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MQTTClient
    )

# Generate version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/MQTTClientConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
    )

# Install config files
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/MQTTClientConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/MQTTClientConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MQTTClient
        )
