message(STATUS "Configuring Sanitizers")

# Function to add sanitizer flags
function(add_sanitizer_flags)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        # Common flags for all sanitizers
        add_compile_options("-fno-omit-frame-pointer")
        add_link_options("-fno-omit-frame-pointer")

        # Address Sanitizer
        if(ENABLE_ADDRESS_SANITIZER)
            message(STATUS "Activating Address Sanitizer")
            add_compile_options("-fsanitize=address")
            add_link_options("-fsanitize=address")
            add_definitions(-DASAN_ENABLED)
        endif()

        # Undefined Behavior Sanitizer
        if(ENABLE_UNDEFINED_SANITIZER)
            message(STATUS "Activating Undefined Behavior Sanitizer")
            add_compile_options("-fsanitize=undefined")
            add_link_options("-fsanitize=undefined")
            add_definitions(-DUBSAN_ENABLED)
        endif()

        # Leak Sanitizer
        if(ENABLE_LEAK_SANITIZER)
            message(STATUS "Activating Leak Sanitizer")
            add_compile_options("-fsanitize=leak")
            add_link_options("-fsanitize=leak")
            add_definitions(-DLSAN_ENABLED)
        endif()

        # Thread Sanitizer
        if(ENABLE_THREAD_SANITIZER)
            if(ENABLE_ADDRESS_SANITIZER OR ENABLE_LEAK_SANITIZER)
                message(WARNING "Thread Sanitizer cannot be used with Address or Leak Sanitizer")
            else()
                message(STATUS "Activating Thread Sanitizer")
                add_compile_options("-fsanitize=thread")
                add_link_options("-fsanitize=thread")
                add_definitions(-DTSAN_ENABLED)
            endif()
        endif()

        # Memory Sanitizer (Clang only)
        if(ENABLE_MEMORY_SANITIZER)
            if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
                if(ENABLE_ADDRESS_SANITIZER OR ENABLE_THREAD_SANITIZER)
                    message(WARNING "Memory Sanitizer cannot be used with Address or Thread Sanitizer")
                else()
                    message(STATUS "Activating Memory Sanitizer")
                    add_compile_options("-fsanitize=memory")
                    add_link_options("-fsanitize=memory")
                    add_definitions(-DMSAN_ENABLED)
                endif()
            else()
                message(WARNING "Memory Sanitizer is only available with Clang")
            endif()
        endif()

    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # MSVC specific sanitizer support
        if(ENABLE_ADDRESS_SANITIZER)
            # Check for incompatible runtime library
            if(CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "MultiThreadedDebug")
                message(
                    FATAL_ERROR
                        "ASan does not support /MDd or /MTd (Debug runtime). Please use /MD or /MT instead."
                    )
            endif()

            # Check for /ZI (Edit and Continue)
            if(CMAKE_CXX_FLAGS MATCHES "/ZI")
                message(FATAL_ERROR "ASan does not support /ZI (Edit and Continue). Use /Zi instead.")
            endif()

            # Check for /LTCG or /GL (Link-Time Code Generation)
            if(CMAKE_CXX_FLAGS MATCHES "/LTCG" OR CMAKE_CXX_FLAGS MATCHES "/GL")
                message(
                    FATAL_ERROR
                        "ASan does not support /LTCG or /GL. Remove them from your configuration."
                    )
            endif()

            # Check for security checks that interfere with ASan
            if(CMAKE_CXX_FLAGS MATCHES "/GS" OR CMAKE_CXX_FLAGS MATCHES "/RTC")
                message(
                    FATAL_ERROR
                        "ASan does not support /GS (Buffer Security Check) or /RTC (Runtime Check). Please remove them."
                    )
            endif()

            # Set the MSVC debug information format to ProgramDatabase to avoid conflicts with ASan
            set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "ProgramDatabase")

            # Ensure incremental linking is disabled (required for ASan)
            add_link_options("/INCREMENTAL:NO")

            message(STATUS "Activating Address Sanitizer (MSVC)")
            add_compile_options("/fsanitize=address")
            add_definitions(-DASAN_ENABLED)
        endif()

        # MSVC doesn't support other sanitizers
        if(ENABLE_UNDEFINED_SANITIZER OR ENABLE_LEAK_SANITIZER OR 
           ENABLE_THREAD_SANITIZER OR ENABLE_MEMORY_SANITIZER)
            message(WARNING "Only Address Sanitizer is supported in MSVC")
        endif()
    else()
        message(WARNING "Sanitizers are not supported for this compiler")
    endif()
endfunction()

# Enable all sanitizers if ENABLE_SANITIZERS is ON
if(ENABLE_SANITIZERS)
    set(ENABLE_ADDRESS_SANITIZER ON)
    set(ENABLE_UNDEFINED_SANITIZER ON)
    set(ENABLE_LEAK_SANITIZER ON)
    set(ENABLE_THREAD_SANITIZER ON)
    set(ENABLE_MEMORY_SANITIZER ON)
endif()

# Add sanitizer-specific compile definitions
if(ENABLE_SANITIZERS OR ENABLE_ADDRESS_SANITIZER OR ENABLE_UNDEFINED_SANITIZER OR 
   ENABLE_LEAK_SANITIZER OR ENABLE_THREAD_SANITIZER OR ENABLE_MEMORY_SANITIZER)
    add_definitions(-DSANITIZERS_ENABLED)
endif()

add_sanitizer_flags()