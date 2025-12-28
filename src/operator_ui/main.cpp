#include "ui_server.hpp"
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
    
    std::string static_dir = "src/operator_ui/static";
    if (argc > 2) {
        static_dir = argv[2];
    }

    auto cfg = config::load(config_path);
    logging::init("ui", cfg.logging.log_dir, cfg.logging.flush_every_n);
    logging::info("Starting operator UI");

    ui::UIServer server{cfg, static_dir};
    server.run();

    while (!g_quit) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    server.stop();
    
    logging::info("Operator UI shutting down");
    logging::shutdown();
    return 0;
}
