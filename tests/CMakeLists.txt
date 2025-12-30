add_library(test_support STATIC
    support/log_parse.cpp
)

if (WIN32)
    target_sources(test_support PRIVATE support/proc_windows.cpp)
else()
    target_sources(test_support PRIVATE support/proc_linux.cpp)
endif()

target_include_directories(test_support PUBLIC support)
target_link_libraries(test_support PUBLIC common Catch2::Catch2WithMain nlohmann_json::nlohmann_json)

include(Catch)

# Latency Test
add_executable(test_latency test_latency.cpp)
target_link_libraries(test_latency PRIVATE test_support)
catch_discover_tests(test_latency)

# Fault Tolerance Test
add_executable(test_fault_tolerance test_fault_tolerance.cpp)
target_link_libraries(test_fault_tolerance PRIVATE test_support)
catch_discover_tests(test_fault_tolerance)

# Determinism Test
add_executable(test_determinism test_determinism.cpp)
target_link_libraries(test_determinism PRIVATE test_support)
catch_discover_tests(test_determinism)

# Logging Test
add_executable(test_logging test_logging.cpp)
target_link_libraries(test_logging PRIVATE test_support)
catch_discover_tests(test_logging)
