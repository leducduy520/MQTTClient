project(app)

add_executable(app "app.cpp")

target_link_libraries(app PRIVATE MQTTClient)
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})