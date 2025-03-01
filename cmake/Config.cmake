# Detect the operating system
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(THIS_OS_WINDOWS 1)
    message(STATUS "WINDOWS OS")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(THIS_OS_LINUX 1)
    message(STATUS "LINUX OS")
endif()

# Detect the compiler being used
if(MSVC)
    # Microsoft Visual Studio (MSVC) detected
    set(THIS_COMPILER_MSVC 1)

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

    # Check if MSVC is being used with Clang
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(THIS_COMPILER_CLANG_CL 1)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Clang compiler detected
    set(THIS_COMPILER_CLANG 1)

    # Extract the Clang version information
    execute_process(
        COMMAND "${CMAKE_CXX_COMPILER}" "--version" OUTPUT_VARIABLE CLANG_VERSION_OUTPUT
        )
    string(REGEX REPLACE ".*clang version ([0-9]+\\.[0-9]+).*" "\\1" THIS_CLANG_VERSION
                         "${CLANG_VERSION_OUTPUT}"
           )

    # Get detailed Clang compiler version information
    execute_process(
        COMMAND "${CMAKE_CXX_COMPILER}" "-v"
        OUTPUT_VARIABLE CLANG_COMPILER_VERSION
        ERROR_VARIABLE CLANG_COMPILER_VERSION
        )
elseif(CMAKE_COMPILER_IS_GNUCXX)
    # GCC compiler detected
    set(THIS_COMPILER_GCC 1)

    # Extract the GCC version
    execute_process(
        COMMAND "${CMAKE_CXX_COMPILER}" "-dumpversion" OUTPUT_VARIABLE GCC_VERSION_OUTPUT
        )
    string(REGEX REPLACE "([0-9]+\\.[0-9]+).*" "\\1" THIS_GCC_VERSION "${GCC_VERSION_OUTPUT}")

    # Get detailed GCC compiler version information
    execute_process(
        COMMAND "${CMAKE_CXX_COMPILER}" "-v"
        OUTPUT_VARIABLE GCC_COMPILER_VERSION
        ERROR_VARIABLE GCC_COMPILER_VERSION
        )
    string(REGEX MATCHALL ".*(tdm[64]*-[1-9]).*" THIS_COMPILER_GCC_TDM "${GCC_COMPILER_VERSION}")

    # Check for UCRT runtime in GCC
    if("${GCC_COMPILER_VERSION}" MATCHES "ucrt")
        set(THIS_RUNTIME_UCRT 1)
    endif()

    # Get the GCC machine architecture
    execute_process(COMMAND "${CMAKE_CXX_COMPILER}" "-dumpmachine" OUTPUT_VARIABLE GCC_MACHINE)
    string(STRIP "${GCC_MACHINE}" GCC_MACHINE)

    # Check for w64-based GCC
    if(GCC_MACHINE MATCHES ".*w64.*")
        set(THIS_COMPILER_GCC_W64 1)
    endif()
endif()

# Set compiler-specific flags based on the detected compiler
if(THIS_COMPILER_GCC OR THIS_COMPILER_CLANG)
    # GCC/Clang-specific flags
    message("CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")
    set(CMAKE_COLOR_DIAGNOSTICS ON) # Enable color diagnostics for GCC/Clang

    # Set common flags for different build types
    set(CMAKE_C_FLAGS_DEBUG_INIT "-g -O0") # Debug build: no optimization, include debug symbols
    set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g -O0")
    set(CMAKE_C_FLAGS_RELEASE_INIT "-O3 -DNDEBUG"
        )# Release build: optimize for performance, disable debugging
    set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O3 -DNDEBUG")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "-O2 -g"
        )# RelWithDebInfo: optimization with debug symbols
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-O2 -g")
    set(CMAKE_C_FLAGS_MINSIZEREL_INIT "-Os -DNDEBUG"
        )# MinSizeRel: optimize for size, disable debugging
    set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "-Os -DNDEBUG")
elseif(THIS_COMPILER_MSVC OR THIS_COMPILER_CLANG_CL)
    # MSVC/Clang-CL-specific flags
    # Set common flags for different build types
    set(CMAKE_C_FLAGS_DEBUG_INIT "/Zi /Od /Ob0 /RTC1"
        )# Debug build: full debug info, no optimization
    set(CMAKE_CXX_FLAGS_DEBUG_INIT "/Zi /Od /Ob0 /RTC1")
    set(CMAKE_C_FLAGS_RELEASE_INIT "/O2 /Ob2 /DNDEBUG") # Release build: optimize for performance
    set(CMAKE_CXX_FLAGS_RELEASE_INIT "/O2 /Ob2 /DNDEBUG")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "/Zi /O2 /Ob2 /DNDEBUG"
        )# RelWithDebInfo: optimization with debug info
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "/Zi /O2 /Ob2 /DNDEBUG")
    set(CMAKE_C_FLAGS_MINSIZEREL_INIT "/O1 /Ob1 /DNDEBUG") # MinSizeRel: optimize for size
    set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "/O1 /Ob1 /DNDEBUG")
endif()
