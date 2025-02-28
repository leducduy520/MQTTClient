find_package(PahoMqttCpp CONFIG REQUIRED)

add_library(MQTTClient STATIC "mqttclient.cpp")

target_link_libraries(MQTTClient PUBLIC PahoMqttCpp::paho-mqttpp3)

target_include_directories(
    MQTTClient PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
    )

set_target_properties(MQTTClient PROPERTIES FOLDER "PingCCU Service")

target_set_warnings(TARGET MQTTClient ENABLE TRUE AS_ERRORS FALSE)
