#pragma once
#include "config.hpp"
#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <httplib.h>

namespace surveillance {
namespace ui {

class UIServer {
public:
    UIServer(const config::AppConfig& cfg, const std::string& static_dir);
    ~UIServer();

    void run();
    void stop();

private:
    void setup_routes();
    std::string read_file_content(const std::string& path);
    std::string read_recent_alerts();

    config::AppConfig cfg_;
    std::string static_dir_;
    httplib::Server svr_;
    
    std::atomic<bool> running_{false};
    std::thread server_thread_;
};

} // namespace ui
} // namespace surveillance
