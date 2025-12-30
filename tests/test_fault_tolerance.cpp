#include <catch2/catch_test_macros.hpp>
#include "proc.hpp"
#include "log_parse.hpp"
#include <filesystem>
#include <thread>
#include <set>

using namespace surveillance;

#if defined(_WIN32)
    const std::string EXT = ".exe";
#else
    const std::string EXT = "";
#endif

TEST_CASE("TC-FT-001: Fault Tolerance Requirement (SR-002)", "[fault_tolerance]") {
    std::string config_path = "../../../config/system_nominal.json";
    
    std::filesystem::remove_all("run_logs");
    std::filesystem::create_directory("run_logs");

    std::vector<std::unique_ptr<proc::Process>> procs;
    
    procs.push_back(std::make_unique<proc::Process>("../central_processor" + EXT, std::vector<std::string>{config_path}));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    procs.push_back(std::make_unique<proc::Process>("../network_emulator" + EXT, std::vector<std::string>{config_path}));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    struct NodeProc {
        std::unique_ptr<proc::Process> proc;
        bool killed;
    };
    std::vector<NodeProc> nodes;

    for (int i = 0; i < 10; ++i) {
        nodes.push_back({
            std::make_unique<proc::Process>("../sensor_node" + EXT, std::vector<std::string>{
                config_path, "sensor_" + std::to_string(i), std::to_string(i)
            }),
            false
        });
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // Kill 2 nodes violently
    nodes[0].proc->terminate();
    nodes[0].killed = true;
    nodes[1].proc->terminate();
    nodes[1].killed = true;

    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    for (auto& n : nodes) {
        if (!n.killed) n.proc->terminate();
        n.proc->wait();
    }
    for (auto& p : procs) {
        p->terminate();
        p->wait();
    }

    auto alerts = log_parse::read_jsonl("run_logs/alerts.jsonl");
    std::set<std::string> alert_event_ids;
    for (const auto& a : alerts) {
        alert_event_ids.insert(a["event_id"].get<std::string>());
    }

    int total_expected = 0;
    int missing = 0;

    for (int i = 2; i < 10; ++i) {
        auto logs = log_parse::read_jsonl("run_logs/sensor_" + std::to_string(i) + ".jsonl");
        for (const auto& l : logs) {
            if (l.contains("message") && l["message"] == "Generated event") {
                total_expected++;
                std::string eid = l["fields"]["event_id"];
                if (alert_event_ids.find(eid) == alert_event_ids.end()) {
                    missing++;
                }
            }
        }
    }

    // Since we kill cleanly within the overall runtime, 
    // network might drop some, but missing fraction from remaining must be <= 5%
    double missing_rate = static_cast<double>(missing) / std::max(1, total_expected);
    REQUIRE(missing_rate <= 0.05);
}
