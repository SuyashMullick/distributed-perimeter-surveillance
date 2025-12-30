cmake_minimum_required(VERSION 3.24)
project(DistributedSurveillanceCpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find Packages
find_package(cppzmq CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(stduuid CONFIG REQUIRED)

# Common settings
add_library(common_options INTERFACE)
target_compile_features(common_options INTERFACE cxx_std_20)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(common_options INTERFACE -Wall -Wextra -Wpedantic)
endif()

# Common Library
add_library(common STATIC
    src/common/config.cpp
    src/common/time.cpp
    src/common/logging.cpp
    src/common/metrics.cpp
    src/common/zmq_utils.cpp
)
target_include_directories(common PUBLIC src/common)
target_link_libraries(common PUBLIC 
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    cppzmq
    stduuid::stduuid
    common_options
)

# Sensor Node
add_executable(sensor_node
    src/sensor_node/main.cpp
    src/sensor_node/sensor_node.cpp
)
target_include_directories(sensor_node PRIVATE src/sensor_node)
target_link_libraries(sensor_node PRIVATE common)

# Network Emulator
add_executable(network_emulator
    src/network_emulator/main.cpp
    src/network_emulator/network_emulator.cpp
)
target_include_directories(network_emulator PRIVATE src/network_emulator)
target_link_libraries(network_emulator PRIVATE common)

# Central Processor
add_executable(central_processor
    src/central_processor/main.cpp
    src/central_processor/central_processor.cpp
)
target_include_directories(central_processor PRIVATE src/central_processor)
target_link_libraries(central_processor PRIVATE common)

# Operator UI
add_executable(operator_ui
    src/operator_ui/main.cpp
    src/operator_ui/ui_server.cpp
)
target_include_directories(operator_ui PRIVATE src/operator_ui)
target_link_libraries(operator_ui PRIVATE common httplib::httplib)

# Testing
enable_testing()
add_subdirectory(tests)
