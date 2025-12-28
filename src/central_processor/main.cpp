#include "central_processor.hpp"
#include "logging.hpp"
#include <iostream>
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

    auto cfg = config::load(config_path);
    logging::init("central", cfg.logging.log_dir, cfg.logging.flush_every_n);
    logging::info("Starting central processor");

    zmq::context_t ctx{1};
    central::CentralProcessor central{cfg, ctx};
    central.run();

    while (!g_quit) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    central.stop();
    
    logging::info("Central processor shutting down");
    logging::shutdown();
    return 0;
}
