# Detect the operating system
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(THIS_OS_WINDOWS 1)
    message(STATUS "Operating System: Windows")
    message(STATUS "System Name: ${CMAKE_SYSTEM_NAME}")
    message(STATUS "System Version: ${CMAKE_SYSTEM_VERSION}")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(THIS_OS_LINUX 1)
    message(STATUS "Operating System: Linux")
    message(STATUS "System Name: ${CMAKE_SYSTEM_NAME}")
    message(STATUS "System Version: ${CMAKE_SYSTEM_VERSION}")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(THIS_OS_MACOS 1)
    message(STATUS "Operating System: macOS")
    message(STATUS "System Name: ${CMAKE_SYSTEM_NAME}")
    message(STATUS "System Version: ${CMAKE_SYSTEM_VERSION}")
endif()

# Detect the compiler being used
if(MSVC)
    # Microsoft Visual Studio (MSVC) detected
    set(THIS_COMPILER_MSVC 1)
    message(STATUS "Compiler: Microsoft Visual Studio (MSVC)")
    message(STATUS "MSVC Version: ${MSVC_VERSION}")

    # Map MSVC version numbers to user-friendly values
    if(MSVC_VERSION EQUAL 1400)
        set(THIS_MSVC_VERSION 8)
    elseif(MSVC_VERSION EQUAL 1500)
        set(THIS_MSVC_VERSION 9)
    elseif(MSVC_VERSION EQUAL 1600)
        set(THIS_MSVC_VERSION 10)
    elseif(MSVC_VERSION EQUAL 1700)
        set(THIS_MSVC_VERSION 11)
    elseif(MSVC_VERSION EQUAL 1800)
        set(THIS_MSVC_VERSION 12)
    elseif(MSVC_VERSION EQUAL 1900)
        set(THIS_MSVC_VERSION 14)
    elseif(MSVC_VERSION LESS_EQUAL 1919)
        set(THIS_MSVC_VERSION 15)
    elseif(MSVC_VERSION LESS_EQUAL 1929)
        set(THIS_MSVC_VERSION 16)
    elseif(MSVC_VERSION LESS_EQUAL 1939)
        set(THIS_MSVC_VERSION 17)
    endif()
    message(STATUS "MSVC User-Friendly Version: ${THIS_MSVC_VERSION}")

    # Check if MSVC is being used with Clang
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(THIS_COMPILER_CLANG_CL 1)
        message(STATUS "Using Clang-CL with MSVC")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Clang compiler detected
    set(THIS_COMPILER_CLANG 1)
    message(STATUS "Compiler: Clang")

    # Extract the Clang version information
    execute_process(
        COMMAND "${CMAKE_CXX_COMPILER}" "--version" OUTPUT_VARIABLE CLANG_VERSION_OUTPUT
        )
    string(REGEX REPLACE ".*clang version ([0-9]+\\.[0-9]+).*" "\\1" THIS_CLANG_VERSION
                         "${CLANG_VERSION_OUTPUT}"
           )
    message(STATUS "Clang Version: ${THIS_CLANG_VERSION}")

    # Get detailed Clang compiler version information
    execute_process(
        COMMAND "${CMAKE_CXX_COMPILER}" "-v"
        OUTPUT_VARIABLE CLANG_COMPILER_VERSION
        ERROR_VARIABLE CLANG_COMPILER_VERSION
        )
    message(STATUS "Clang Compiler Details: ${CLANG_COMPILER_VERSION}")
elseif(CMAKE_COMPILER_IS_GNUCXX)
    # GCC compiler detected
    set(THIS_COMPILER_GCC 1)
    message(STATUS "Compiler: GCC")

    # Extract the GCC version
    execute_process(
        COMMAND "${CMAKE_CXX_COMPILER}" "-dumpversion" OUTPUT_VARIABLE GCC_VERSION_OUTPUT
        )
    string(REGEX REPLACE "([0-9]+\\.[0-9]+).*" "\\1" THIS_GCC_VERSION "${GCC_VERSION_OUTPUT}")
    message(STATUS "GCC Version: ${THIS_GCC_VERSION}")

    # Get detailed GCC compiler version information
    execute_process(
        COMMAND "${CMAKE_CXX_COMPILER}" "-v"
        OUTPUT_VARIABLE GCC_COMPILER_VERSION
        ERROR_VARIABLE GCC_COMPILER_VERSION
        )
    message(STATUS "GCC Compiler Details: ${GCC_COMPILER_VERSION}")
    string(REGEX MATCHALL ".*(tdm[64]*-[1-9]).*" THIS_COMPILER_GCC_TDM "${GCC_COMPILER_VERSION}")

    # Check for UCRT runtime in GCC
    if("${GCC_COMPILER_VERSION}" MATCHES "ucrt")
        set(THIS_RUNTIME_UCRT 1)
        message(STATUS "Using UCRT runtime")
    endif()

    # Get the GCC machine architecture
    execute_process(COMMAND "${CMAKE_CXX_COMPILER}" "-dumpmachine" OUTPUT_VARIABLE GCC_MACHINE)
    string(STRIP "${GCC_MACHINE}" GCC_MACHINE)
    message(STATUS "GCC Target Machine: ${GCC_MACHINE}")

    # Check for w64-based GCC
    if(GCC_MACHINE MATCHES ".*w64.*")
        set(THIS_COMPILER_GCC_W64 1)
        message(STATUS "Using w64-based GCC")
    endif()
endif()

# C++ Standard Version Detection and Configuration
if(THIS_COMPILER_MSVC OR THIS_COMPILER_CLANG_CL)
    # MSVC/Clang-CL C++ standard detection
    if(MSVC_VERSION GREATER_EQUAL 1930)
        set(THIS_CPP_STANDARD 20)
    elseif(MSVC_VERSION GREATER_EQUAL 1910)
        set(THIS_CPP_STANDARD 17)
    elseif(MSVC_VERSION GREATER_EQUAL 1900)
        set(THIS_CPP_STANDARD 14)
    else()
        set(THIS_CPP_STANDARD 11)
    endif()
    message(STATUS "MSVC/Clang-CL C++ Standard: ${THIS_CPP_STANDARD}")
elseif(THIS_COMPILER_CLANG)
    # Clang C++ standard detection
    if(THIS_CLANG_VERSION VERSION_GREATER_EQUAL 15.0)
        set(THIS_CPP_STANDARD 20)
    elseif(THIS_CLANG_VERSION VERSION_GREATER_EQUAL 5.0)
        set(THIS_CPP_STANDARD 17)
    elseif(THIS_CLANG_VERSION VERSION_GREATER_EQUAL 3.4)
        set(THIS_CPP_STANDARD 14)
    else()
        set(THIS_CPP_STANDARD 11)
    endif()
    message(STATUS "Clang C++ Standard: ${THIS_CPP_STANDARD}")
elseif(THIS_COMPILER_GCC)
    # GCC C++ standard detection
    if(THIS_GCC_VERSION VERSION_GREATER_EQUAL 11.0)
        set(THIS_CPP_STANDARD 20)
    elseif(THIS_GCC_VERSION VERSION_GREATER_EQUAL 7.0)
        set(THIS_CPP_STANDARD 17)
    elseif(THIS_GCC_VERSION VERSION_GREATER_EQUAL 5.0)
        set(THIS_CPP_STANDARD 14)
    else()
        set(THIS_CPP_STANDARD 11)
    endif()
    message(STATUS "GCC C++ Standard: ${THIS_CPP_STANDARD}")
endif()

# Set C++ standard based on detected version
if(THIS_CPP_STANDARD EQUAL 20)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
elseif(THIS_CPP_STANDARD EQUAL 17)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
elseif(THIS_CPP_STANDARD EQUAL 14)
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
else()
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

message(STATUS "Using C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "C++ Standard Required: ${CMAKE_CXX_STANDARD_REQUIRED}")
message(STATUS "C++ Extensions Disabled: ${CMAKE_CXX_EXTENSIONS}")

# Set compiler-specific flags based on the detected compiler
if(THIS_COMPILER_GCC OR THIS_COMPILER_CLANG)
    # GCC/Clang-specific flags
    message(STATUS "Configuring GCC/Clang compiler flags")
    message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
    set(CMAKE_COLOR_DIAGNOSTICS ON)
    message(STATUS "Color Diagnostics: Enabled")

    # Set common flags for different build types
    set(CMAKE_C_FLAGS_DEBUG_INIT "-g -O0")
    set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g -O0")
    message(STATUS "Debug Flags: -g -O0")

    set(CMAKE_C_FLAGS_RELEASE_INIT "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O3 -DNDEBUG")
    message(STATUS "Release Flags: -O3 -DNDEBUG")

    set(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "-O2 -g")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-O2 -g")
    message(STATUS "RelWithDebInfo Flags: -O2 -g")

    set(CMAKE_C_FLAGS_MINSIZEREL_INIT "-Os -DNDEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "-Os -DNDEBUG")
    message(STATUS "MinSizeRel Flags: -Os -DNDEBUG")
elseif(THIS_COMPILER_MSVC OR THIS_COMPILER_CLANG_CL)
    # MSVC/Clang-CL-specific flags
    message(STATUS "Configuring MSVC/Clang-CL compiler flags")
    message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

    # Set common flags for different build types
    set(CMAKE_C_FLAGS_DEBUG_INIT "/Zi /Od /Ob0 /RTC1")
    set(CMAKE_CXX_FLAGS_DEBUG_INIT "/Zi /Od /Ob0 /RTC1")
    message(STATUS "Debug Flags: /Zi /Od /Ob0 /RTC1")

    set(CMAKE_C_FLAGS_RELEASE_INIT "/O2 /Ob2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE_INIT "/O2 /Ob2 /DNDEBUG")
    message(STATUS "Release Flags: /O2 /Ob2 /DNDEBUG")

    set(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "/Zi /O2 /Ob2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "/Zi /O2 /Ob2 /DNDEBUG")
    message(STATUS "RelWithDebInfo Flags: /Zi /O2 /Ob2 /DNDEBUG")

    set(CMAKE_C_FLAGS_MINSIZEREL_INIT "/O1 /Ob1 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "/O1 /Ob1 /DNDEBUG")
    message(STATUS "MinSizeRel Flags: /O1 /Ob1 /DNDEBUG")
endif()

# Architecture-specific optimizations
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(THIS_ARCH_X86_64 1)
    message(STATUS "Architecture: x86_64/AMD64")
    if(THIS_COMPILER_GCC OR THIS_COMPILER_CLANG)
        set(ARCH_FLAGS "-march=native -mtune=native")
        message(STATUS "GCC/Clang Architecture Flags: ${ARCH_FLAGS}")
    elseif(THIS_COMPILER_MSVC)
        set(ARCH_FLAGS "/arch:AVX2")
        message(STATUS "MSVC Architecture Flags: ${ARCH_FLAGS}")
    endif()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64")
    set(THIS_ARCH_ARM64 1)
    message(STATUS "Architecture: ARM64")
    if(THIS_COMPILER_GCC OR THIS_COMPILER_CLANG)
        set(ARCH_FLAGS "-march=native -mtune=native")
        message(STATUS "GCC/Clang Architecture Flags: ${ARCH_FLAGS}")
    endif()
endif()

# Cross-compilation support
if(CMAKE_CROSSCOMPILING)
    message(STATUS "Cross-compiling for ${CMAKE_SYSTEM_NAME}")
    message(STATUS "Host System: ${CMAKE_HOST_SYSTEM_NAME}")
    message(STATUS "Target System: ${CMAKE_SYSTEM_NAME}")
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
        set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
        set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
        set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
        message(STATUS "Cross-compilation root path modes configured for Linux")
    endif()
endif()

message(STATUS "Configuration completed successfully")
