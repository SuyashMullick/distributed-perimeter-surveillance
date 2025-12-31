#include <catch2/catch_test_macros.hpp>
#include "proc.hpp"
#include <filesystem>
#include <thread>
#include <fstream>
#include <sstream>

using namespace surveillance;

#if defined(_WIN32)
    const std::string EXT = ".exe";
#else
    const std::string EXT = "";
#endif

std::string read_file_content(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return "";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

void run_deterministic_cycle(const std::string& out_dir) {
    std::string config_path = "../../../config/system_deterministic.json";
    
    std::filesystem::remove_all("run_logs");
    std::filesystem::create_directory("run_logs");

    std::vector<std::unique_ptr<proc::Process>> procs;
    
    // Index 0: central processor
    procs.push_back(std::make_unique<proc::Process>("../central_processor" + EXT, std::vector<std::string>{config_path}));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Index 1: network emulator
    procs.push_back(std::make_unique<proc::Process>("../network_emulator" + EXT, std::vector<std::string>{config_path}));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Index 2+: sensor nodes
    for (int i = 0; i < 1; ++i) {
        procs.push_back(std::make_unique<proc::Process>("../sensor_node" + EXT, std::vector<std::string>{
            config_path, "sensor_" + std::to_string(i), std::to_string(i)
        }));
    }

    // Sensor runs in deterministic mode (no real-time sleep), so it finishes quickly.
    // Wait long enough for sensor to complete all events.
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Staged shutdown: kill sensors first
    for (size_t i = 2; i < procs.size(); ++i) {
        procs[i]->terminate();
        procs[i]->wait();
    }

    // Wait for network emulator to flush its queue to central
    std::this_thread::sleep_for(std::chrono::seconds(2));
    procs[1]->terminate();
    procs[1]->wait();

    // Wait for central processor to drain the ZMQ receive queue and write all alerts
    std::this_thread::sleep_for(std::chrono::seconds(3));
    procs[0]->terminate();
    procs[0]->wait();
    
    std::filesystem::remove_all(out_dir);
    std::filesystem::copy("run_logs", out_dir, std::filesystem::copy_options::recursive);
}

TEST_CASE("TC-DET-001: Determinism Requirement (SR-005)", "[determinism]") {
    run_deterministic_cycle("run_1_logs");

    // Give the OS time to fully release ZMQ ports before run 2 rebinds them
    std::this_thread::sleep_for(std::chrono::seconds(2));

    run_deterministic_cycle("run_2_logs");
    
    std::string alerts1 = read_file_content("run_1_logs/alerts.jsonl");
    std::string alerts2 = read_file_content("run_2_logs/alerts.jsonl");
    
    REQUIRE(!alerts1.empty());
    REQUIRE(alerts1 == alerts2);
}
