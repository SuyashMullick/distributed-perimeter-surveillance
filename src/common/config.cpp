#include "config.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace surveillance {
namespace config {

using nlohmann::json;

AppConfig load(const std::string& path) {
    AppConfig cfg;
    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error("Failed to open config file: " + path);
    }
    
    json j;
    f >> j;

    if (j.contains("system")) {
        auto& s = j["system"];
        if (s.contains("mode")) cfg.system.mode = s["mode"];
        if (s.contains("duration_s")) cfg.system.duration_s = s["duration_s"];
        if (s.contains("num_nodes")) cfg.system.num_nodes = s["num_nodes"];
        if (s.contains("seed_base")) cfg.system.seed_base = s["seed_base"];
    }

    if (j.contains("sensor")) {
        auto& s = j["sensor"];
        if (s.contains("event_rate_hz")) cfg.sensor.event_rate_hz = s["event_rate_hz"];
        if (s.contains("status_rate_hz")) cfg.sensor.status_rate_hz = s["status_rate_hz"];
    }

    if (j.contains("network")) {
        auto& s = j["network"];
        if (s.contains("latency_ms")) cfg.network.latency_ms = s["latency_ms"];
        if (s.contains("jitter_ms")) cfg.network.jitter_ms = s["jitter_ms"];
        if (s.contains("loss_rate")) cfg.network.loss_rate = s["loss_rate"];
        if (s.contains("network_seed")) cfg.network.network_seed = s["network_seed"];
        if (s.contains("reorder_enabled")) cfg.network.reorder_enabled = s["reorder_enabled"];
    }

    if (j.contains("central")) {
        auto& s = j["central"];
        if (s.contains("heartbeat_timeout_s")) cfg.central.heartbeat_timeout_s = s["heartbeat_timeout_s"];
        if (s.contains("alerts_buffer")) cfg.central.alerts_buffer = s["alerts_buffer"];
    }

    if (j.contains("logging")) {
        auto& s = j["logging"];
        if (s.contains("log_dir")) cfg.logging.log_dir = s["log_dir"];
        if (s.contains("flush_every_n")) cfg.logging.flush_every_n = s["flush_every_n"];
    }

    return cfg;
}

} // namespace config
} // namespace surveillance
