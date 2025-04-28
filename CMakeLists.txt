cmake_minimum_required(VERSION 3.25)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified")
    set(CMAKE_BUILD_TYPE
        RelWithDebInfo
        CACHE STRING "Choose the type of build" FORCE
        )
    set_property(
        CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
        )
endif()

# Check if compiler supports C++17
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.14)
    message(FATAL_ERROR "MSVC 19.14 or higher is required for C++17 support")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
    message(FATAL_ERROR "GCC 7.0 or higher is required for C++17 support")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
    message(FATAL_ERROR "Clang 5.0 or higher is required for C++17 support")
endif()

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
# Set project version
project(
    "MQTTClient"
    VERSION 1.0.0
    LANGUAGES CXX
    )

# Include built-in modules
include(GNUInstallDirs)

# Project options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(ENABLE_CMAKE_FORMAT "Enable CMake Format" OFF)
option(ENABLE_CLANG_FORMAT "Enable Clang Format" OFF)
option(ENABLE_TESTING "Enable unit tests with GoogleTest" OFF)

# Define sanitizer options
option(ENABLE_SANITIZERS "Enable all sanitizers" OFF)
option(ENABLE_ADDRESS_SANITIZER "Enable Address Sanitizer" OFF)
option(ENABLE_UNDEFINED_SANITIZER "Enable Undefined Behavior Sanitizer" OFF)
option(ENABLE_LEAK_SANITIZER "Enable Leak Sanitizer" OFF)
option(ENABLE_THREAD_SANITIZER "Enable Thread Sanitizer" OFF)
option(ENABLE_MEMORY_SANITIZER "Enable Memory Sanitizer" OFF)

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

# Configure startup parameters
include(Config)

# Enable CMake Format and Clang Format if specified
if(ENABLE_CMAKE_FORMAT OR ENABLE_CLANG_FORMAT)
    include(Tools)

    if(ENABLE_CMAKE_FORMAT)
        add_cmake_format("(out|vcpkg)")
    endif()

    if(ENABLE_CLANG_FORMAT)
        add_clang_format("(out|vcpkg)")
    endif()
endif()

# Apply sanitizer flags
include(Sanitizer)

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

set(CPACK_PACKAGE_NAME "CPP-MQTTClient")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION "SDK for using mqtt in more convenient way")
set(CPACK_PACKAGE_VENDOR "Le Duc Duy")
set(CPACK_PACKAGE_CONTACT "duyld520@email.com")
set(CPACK_GENERATOR "ZIP;TGZ")
set(CPACK_INSTALL_CMAKE_PROJECTS
    "mqttclient;mqttclient;Development;/"
)

include(CPack)