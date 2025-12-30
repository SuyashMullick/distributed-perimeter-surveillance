#pragma once
#include <string>
#include <cstdint>

namespace surveillance {
namespace config {

struct SystemConfig {
    std::string mode{"live"};
    int duration_s{600};
    int num_nodes{10};
    uint32_t seed_base{1000};
};

struct SensorConfig {
    double event_rate_hz{0.5};
    double status_rate_hz{1.0};
};

struct NetworkConfig {
    int latency_ms{20};
    int jitter_ms{5};
    double loss_rate{0.001};
    uint32_t network_seed{4242};
    bool reorder_enabled{false};
};

struct CentralConfig {
    double heartbeat_timeout_s{3.0};
    int alerts_buffer{100};
};

struct LoggingConfig {
    std::string log_dir{"run_logs"};
    int flush_every_n{1};
};

struct AppConfig {
    SystemConfig system;
    SensorConfig sensor;
    NetworkConfig network;
    CentralConfig central;
    LoggingConfig logging;
};

// Loads from file and returns config object. Throws on error.
AppConfig load(const std::string& path);

} // namespace config
} // namespace surveillance
