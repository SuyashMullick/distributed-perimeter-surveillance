#include "sensor_node.hpp"
#include "logging.hpp"
#include "ids.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include <csignal>
#include <thread>

using namespace surveillance;

std::atomic<bool> g_quit{false};

void signal_handler(int) {
    g_quit = true;
}

int main(int argc, char** argv) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::string config_path = "config/system_nominal.json";
    if (argc > 1) {
        config_path = argv[1];
    }
    std::string node_id = "sensor_0";
    int node_index = 0;
    if (argc > 3) {
        node_id = argv[2];
        node_index = std::stoi(argv[3]);
    }

    auto cfg = config::load(config_path);
    if (cfg.system.mode == "deterministic") {
        ids::seed(cfg.system.seed_base + node_index);
    }
    
    logging::init(node_id, cfg.logging.log_dir, cfg.logging.flush_every_n);
    logging::info("Starting sensor node", {{"node_id", node_id}, {"index", node_index}});

    zmq::context_t ctx{1};
    sensor::SensorNode node{node_id, node_index, cfg, ctx};

    std::thread t([&node, &cfg]() {
        if (cfg.system.mode == "deterministic") {
            node.run_deterministic();
        } else {
            node.run_live();
        }
        g_quit = true;
    });

    while (!g_quit) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    node.stop();
    t.join();
    
    logging::info("Sensor node shutting down");
    logging::shutdown();
    return 0;
}
