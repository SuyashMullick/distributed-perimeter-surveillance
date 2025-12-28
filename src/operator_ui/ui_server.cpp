#include "ui_server.hpp"
#include "logging.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

namespace surveillance {
namespace ui {

UIServer::UIServer(const config::AppConfig& cfg, const std::string& static_dir)
    : cfg_(cfg), static_dir_(static_dir)
{
    setup_routes();
}

UIServer::~UIServer() {
    stop();
}

void UIServer::stop() {
    if (running_) {
        running_ = false;
        svr_.stop();
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }
}

void UIServer::run() {
    if (!svr_.set_mount_point("/static", static_dir_)) {
        logging::error("Failed to mount static directory: " + static_dir_);
    }
    
    running_ = true;
    server_thread_ = std::thread([this]() {
        logging::info("Operator UI server listening on http://127.0.0.1:8080");
        if (!svr_.listen("127.0.0.1", 8080)) {
            logging::error("Failed to bind UI server to port 8080");
        }
        running_ = false;
    });
}

std::string UIServer::read_file_content(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return "";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::string UIServer::read_recent_alerts() {
    // Read the tail of alerts.jsonl safely. Since central processor replaces or appends,
    // we simply read the file. If it gets big, a tail approach is better.
    // For this simulation reading it bounded is fine.
    std::ifstream f(cfg_.logging.log_dir + "/alerts.jsonl");
    if (!f.is_open()) return "[]";
    
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty()) lines.push_back(line);
    }
    
    size_t count = std::min(lines.size(), static_cast<size_t>(100));
    std::ostringstream ss;
    ss << "[";
    for (size_t i = lines.size() - count; i < lines.size(); ++i) {
        ss << lines[i] << (i + 1 == lines.size() ? "" : ",");
    }
    ss << "]";
    return ss.str();
}

void UIServer::setup_routes() {
    svr_.Get("/", [this](const httplib::Request&, httplib::Response& res) {
        std::string content = read_file_content(static_dir_ + "/index.html");
        res.set_content(content, "text/html");
    });

    svr_.Get("/api/status", [this](const httplib::Request&, httplib::Response& res) {
        std::string content = read_file_content(cfg_.logging.log_dir + "/central_state.json");
        if (content.empty()) {
            content = "{}";
        }
        res.set_content(content, "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    svr_.Get("/api/alerts", [this](const httplib::Request&, httplib::Response& res) {
        std::string content = read_recent_alerts();
        res.set_content(content, "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
    });
}

} // namespace ui
} // namespace surveillance
