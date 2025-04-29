# Default target
all: help

# Format configuration
EXCLUDE_FOLDERS_REGEX ?= (out|vcpkg)

# Detect OS and set vcpkg root
ifeq ($(OS),Windows_NT)
    # Windows
    VCPKG_ROOT ?= $(shell powershell.exe -Command "if (Test-Path env:VCPKG_ROOT) { Write-Output $env:VCPKG_ROOT } elseif (Test-Path 'vcpkg') { Write-Output '$(CURDIR)/vcpkg' } else { Write-Output 'C:/vcpkg' }")
    VCPKG_TRIPLET ?= x64-windows
    VCPKG_TOOLCHAIN_FILE := $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
    SHELL := powershell.exe
    .SHELLFLAGS := -NoProfile -Command
    CHECK_VCPKG_CMD := if (!(Test-Path "$(VCPKG_TOOLCHAIN_FILE)")) { \
        if (Test-Path "vcpkg") { \
            if (Test-Path "vcpkg/.git") { \
                Write-Host "Initializing vcpkg submodule..."; \
                git submodule update --init --recursive vcpkg; \
            } else { \
                Write-Host "Cloning vcpkg..."; \
                git clone https://github.com/Microsoft/vcpkg.git; \
                Set-Location vcpkg; \
                ./bootstrap-vcpkg.bat; \
            } \
        } else { \
            Write-Error "Error: vcpkg not found at $(VCPKG_ROOT)"; \
            Write-Error "Please install vcpkg or set VCPKG_ROOT environment variable"; \
            exit 1; \
        } \
    }
else
    # Linux/macOS
    VCPKG_ROOT ?= $(shell if [ -n "$$VCPKG_ROOT" ]; then echo "$$VCPKG_ROOT"; elif [ -d "vcpkg" ]; then echo "$(CURDIR)/vcpkg"; else echo "/usr/local/vcpkg"; fi)
    VCPKG_TRIPLET ?= x64-linux
    VCPKG_TOOLCHAIN_FILE := $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
    SHELL := /bin/bash
    .SHELLFLAGS := -c
    CHECK_VCPKG_CMD := if [ ! -f "$(VCPKG_TOOLCHAIN_FILE)" ]; then \
        if [ -d "vcpkg" ]; then \
            if [ -d "vcpkg/.git" ]; then \
                echo "Initializing vcpkg submodule..."; \
                git submodule update --init --recursive vcpkg; \
            else \
                echo "Cloning vcpkg..."; \
                git clone https://github.com/Microsoft/vcpkg.git; \
                cd vcpkg && ./bootstrap-vcpkg.sh; \
            fi \
        else \
            echo "Error: vcpkg not found at $(VCPKG_ROOT)"; \
            echo "Please install vcpkg or set VCPKG_ROOT environment variable"; \
            exit 1; \
        fi \
    fi
endif

# Check if vcpkg is installed
check-vcpkg:
	@$(CHECK_VCPKG_CMD)

# Initialize environment
init-env: check-vcpkg
	@echo "Initializing environment..."
	@echo "OS: $(OS)"
	@echo "VCPKG_ROOT: $(VCPKG_ROOT)"
	@echo "VCPKG_TRIPLET: $(VCPKG_TRIPLET)"
	@echo "VCPKG_TOOLCHAIN_FILE: $(VCPKG_TOOLCHAIN_FILE)"
	@echo "Environment initialized successfully"

# Build commands
build-debug-x64:
	cmake --build --preset x64-debug

build-release-x64:
	cmake --build --preset x64-release

build-debug-x86:
	cmake --build --preset x86-debug

build-release-x86:
	cmake --build --preset x86-release

build-debug-linux:
	cmake --build --preset linux-debug

build-release-linux:
	cmake --build --preset linux-release

# Test commands
test-debug-x64:
	ctest --preset x64-debug

test-release-x64:
	ctest --preset x64-release

test-debug-x86:
	ctest --preset x86-debug

test-release-x86:
	ctest --preset x86-release

test-debug-linux:
	ctest --preset linux-debug

test-release-linux:
	ctest --preset linux-release

# Format commands
format-cmake:
	@echo "Running cmake-format..."
	@echo "Finding CMake files..."
	@find . -name "CMakeLists.txt" -o -name "*.cmake" | grep -E "(./vcpkg|./out)" -v | while read file; do \
		echo "Formatting: $$file"; \
		cmake-format -c .cmake-format.yaml -i "$$file"; \
	done

format-clang:
	@echo "Running clang-format..."
	@echo "Finding C/C++ files..."
	@find . -name "*.cc" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | grep -E "(./vcpkg|./out)" -v | while read file; do \
		echo "Formatting: $$file"; \
		clang-format -i "$$file"; \
	done

format: format-cmake format-clang

# Configure commands
configure-debug-x64:
	cmake --preset x64-debug

configure-release-x64:
	cmake --preset x64-release

configure-debug-x86:
	cmake --preset x86-debug

configure-release-x86:
	cmake --preset x86-release

configure-debug-linux:
	cmake --preset linux-debug

configure-release-linux:
	cmake --preset linux-release

# Clean commands
clean-debug-x64:
	cmake --build --preset x64-debug --target clean

clean-release-x64:
	cmake --build --preset x64-release --target clean

clean-debug-x86:
	cmake --build --preset x86-debug --target clean

clean-release-x86:
	cmake --build --preset x86-release --target clean

clean-debug-linux:
	cmake --build --preset linux-debug --target clean

clean-release-linux:
	cmake --build --preset linux-release --target clean

# Package commands
package-release-x64:
	cmake --build --preset x64-release --target package

package-release-linux:
	cmake --build --preset linux-release --target package

# Workflow commands
workflow-deploy-linux:
	cmake --workflow --preset deloy-linux

workflow-deploy-x64:
	cmake --workflow --preset deploy-x64

# Help command
help:
	@echo "Available commands:"
	@echo "  Build commands:"
	@echo "    build-debug-x64     - Build x64 Debug"
	@echo "    build-release-x64   - Build x64 Release"
	@echo "    build-debug-x86     - Build x86 Debug"
	@echo "    build-release-x86   - Build x86 Release"
	@echo "    build-debug-linux   - Build Linux Debug"
	@echo "    build-release-linux - Build Linux Release"
	@echo ""
	@echo "  Test commands:"
	@echo "    test-debug-x64     - Run x64 Debug tests"
	@echo "    test-release-x64   - Run x64 Release tests"
	@echo "    test-debug-x86     - Run x86 Debug tests"
	@echo "    test-release-x86   - Run x86 Release tests"
	@echo "    test-debug-linux   - Run Linux Debug tests"
	@echo "    test-release-linux - Run Linux Release tests"
	@echo ""
	@echo "  Format commands:"
	@echo "    format-cmake      - Format CMake files using cmake-format"
	@echo "    format-clang      - Format C/C++ files using clang-format"
	@echo "    format           - Run all formatters"
	@echo ""
	@echo "  Configure commands:"
	@echo "    configure-debug-x64     - Configure x64 Debug"
	@echo "    configure-release-x64   - Configure x64 Release"
	@echo "    configure-debug-x86     - Configure x86 Debug"
	@echo "    configure-release-x86   - Configure x86 Release"
	@echo "    configure-debug-linux   - Configure Linux Debug"
	@echo "    configure-release-linux - Configure Linux Release"
	@echo ""
	@echo "  Clean commands:"
	@echo "    clean-debug-x64     - Clean x64 Debug"
	@echo "    clean-release-x64   - Clean x64 Release"
	@echo "    clean-debug-x86     - Clean x86 Debug"
	@echo "    clean-release-x86   - Clean x86 Release"
	@echo "    clean-debug-linux   - Clean Linux Debug"
	@echo "    clean-release-linux - Clean Linux Release"
	@echo ""
	@echo "  Package commands:"
	@echo "    package-release-x64   - Package x64 Release"
	@echo "    package-release-linux - Package Linux Release"
	@echo ""
	@echo "  Workflow commands:"
	@echo "    workflow-deploy-linux - Run Linux deployment workflow"
	@echo "    workflow-deploy-x64   - Run Windows deployment workflow"

