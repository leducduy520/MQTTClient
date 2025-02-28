cmake_minimum_required(VERSION 3.25)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add custom CMake modules path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Enable Hot Reload for MSVC compilers if supported
if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
        "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>"
        )
endif()

# Project name
project("MQTTClient")

# Include built-in modules
include(GNUInstallDirs)

# Project options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(ENABLE_CMAKE_FORMAT "Enable CMake Format" ON)
option(ENABLE_CLANG_FORMAT "Enable Clang Format" ON)
option(ENABLE_TESTING "Enable Testing" OFF)
option(ENABLE_ADDRESS_SANITIZER "Enable Address Sanitizer" ON)
option(ENABLE_UNDEFINED_SANITIZER "Enable Undefined Sanitizer" OFF)
option(ENABLE_LEAK_SANITIZER "Enable Leak Sanitizer" OFF)
option(ENABLE_THREAD_SANITIZER "Enable Thread Sanitizer" OFF)
option(ENABLE_LTO "Enable Link Time Optimization" OFF)

# Generate position-independent code (-fPIC on UNIX)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# Generate compile_commands.json for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Set message log level
set(CMAKE_MESSAGE_LOG_LEVEL VERBOSE)
# Set verbose makefile output
set(CMAKE_VERBOSE_MAKEFILE OFF)

# Enable Link Time Optimization if specified
if(ENABLE_LTO)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Enable CMake Format and Clang Format if specified
if(ENABLE_CMAKE_FORMAT OR ENABLE_CLANG_FORMAT)
    include(Tools)
    if(ENABLE_CMAKE_FORMAT)
        add_cmake_format("out")
    endif()
    if(ENABLE_CLANG_FORMAT)
        add_clang_format("out")
    endif()
endif()

# Enable sanitizers if specified
if(ENABLE_ADDRESS_SANITIZER
   OR ENABLE_UNDEFINED_SANITIZER
   OR ENABLE_LEAK_SANITIZER
   OR ENABLE_THREAD_SANITIZER
   )
    include(Sanitizer)
endif()

# Enable compiler warnings
include(Warnings)

# Add common compile definitions
# Check if the current compiler is Microsoft Visual Studio
if(MSVC)
    # Define _CRT_SECURE_NO_WARNINGS to disable warnings about the use of potentially unsafe C standard library functions
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Add subdirectories for building
add_subdirectory("mqttclient")
add_subdirectory("app")

# Set the startup project for Visual Studio
set_directory_properties(PROPERTIES VS_STARTUP_PROJECT "app")

# Add testing subdirectory if testing is enabled
if(ENABLE_TESTING)
    enable_testing()
    add_subdirectory("test")
endif(ENABLE_TESTING)
