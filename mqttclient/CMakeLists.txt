project(mqttclient VERSION 0.1.0)

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
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Development
    INCLUDES
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

# Install header files
install(
    FILES "mqttclient.hpp" "monitor.hpp"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    COMPONENT Development
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
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/MQTTClientConfig.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/MQTTClientConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MQTTClient
    COMPONENT Development
    )

set(CPACK_PACKAGE_NAME "CPP-MQTTClient")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION "SDK for using mqtt in more convenient way")
set(CPACK_PACKAGE_VENDOR "Le Duc Duy")
set(CPACK_PACKAGE_CONTACT "duyld520@email.com")
set(CPACK_GENERATOR "ZIP;TGZ")

include(CPack)
