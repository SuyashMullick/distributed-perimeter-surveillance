#include <catch2/catch_test_macros.hpp>
#include "proc.hpp"
#include "log_parse.hpp"
#include <filesystem>
#include <thread>

using namespace surveillance;

#if defined(_WIN32)
    const std::string EXT = ".exe";
#else
    const std::string EXT = "";
#endif

TEST_CASE("TC-LOG-001: Logging Completeness Requirement (SR-003)", "[logging]") {
    std::string config_path = "../../../config/system_nominal.json";
    
    std::filesystem::remove_all("run_logs");
    std::filesystem::create_directory("run_logs");

    std::vector<std::unique_ptr<proc::Process>> procs;
    
    procs.push_back(std::make_unique<proc::Process>("../central_processor" + EXT, std::vector<std::string>{config_path}));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    procs.push_back(std::make_unique<proc::Process>("../sensor_node" + EXT, std::vector<std::string>{
        config_path, "sensor_0", "0"
    }));

    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    for (auto& p : procs) {
        p->terminate();
        p->wait();
    }

    auto alerts = log_parse::read_jsonl("run_logs/alerts.jsonl");
    for (const auto& a : alerts) {
        REQUIRE(a.contains("timestamp_utc"));
        REQUIRE(a.contains("source_node_id"));
        REQUIRE(a.contains("event_id"));
        REQUIRE(a.contains("classification"));
        REQUIRE(a.contains("processing_latency_ms"));
    }
}
