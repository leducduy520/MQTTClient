# Define the CMake version to be installed
CMAKE_VERSION ?= 4.0.0
CMAKE_TAR = cmake-$(CMAKE_VERSION).tar.gz
CMAKE_DIR = cmake-$(CMAKE_VERSION)
INSTALL_DIR = /usr/local

# Define the URL to download the CMake source
CMAKE_URL = https://github.com/Kitware/CMake/releases/download/v$(CMAKE_VERSION)/$(CMAKE_TAR)

# Define the required dependencies
DEPS = build-essential libssl-dev wget zip unzip ninja-build pkg-config doxygen graphviz cmake-format clang-format

# Step 1: Install dependencies
install-deps:
	sudo apt-get update
	sudo apt-get install -y $(DEPS)

# Step 2: Download CMake source code
download-cmake:
	wget $(CMAKE_URL)

# Step 3: Extract the tarball
extract-cmake:
	tar -zxvf $(CMAKE_TAR)

# Step 4: Build CMake from source
build-cmake:
	cd $(CMAKE_DIR) && ./bootstrap --parallel=8 && make -j 8

# Step 5: Install CMake
install-cmake:
	sudo make install

# Step 6: Clean up the downloaded files
clean-cmake:
	cd .. && rm -rf $(CMAKE_TAR) $(CMAKE_DIR)

# Step 7: Install CMake (Run all steps)
install: install-deps download-cmake extract-cmake build-cmake install-cmake clean

mosquitto-container:
	docker run -d --name mosquitto -p 30520:1883\
	 -v "$(PWD)/docker/mosquitto/config:/mosquitto/config"\
	  -v "$(PWD)/docker/mosquitto/data:/mosquitto/data"\
	   -v "$(PWD)/docker/mosquitto/log:/mosquitto/log"\
	    eclipse-mosquitto:latest /usr/sbin/mosquitto -c /mosquitto/config/mosquitto.conf -v