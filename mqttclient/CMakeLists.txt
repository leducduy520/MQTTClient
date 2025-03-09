find_package(PahoMqttCpp CONFIG REQUIRED)

add_library(MQTTClient STATIC "mqttclient.cpp")

find_package(PahoMqttCpp CONFIG REQUIRED)
target_link_libraries(MQTTClient PUBLIC $<IF:$<STREQUAL:$<PLATFORM_ID>,Linux>,PahoMqttCpp::paho-mqttpp3-static,PahoMqttCpp::paho-mqttpp3>)
target_include_directories(
    MQTTClient PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
)

set_target_properties(MQTTClient PROPERTIES FOLDER "PingCCU Service")

target_set_warnings(TARGET MQTTClient ENABLE TRUE AS_ERRORS FALSE)

install(TARGETS MQTTClient LIBRARY DESTINATION . ARCHIVE DESTINATION .)