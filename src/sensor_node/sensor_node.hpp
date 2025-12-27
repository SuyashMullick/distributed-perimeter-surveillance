#pragma once

#include "config.hpp"
#include <zmq.hpp>
#include <random>
#include <atomic>
#include <string>

namespace surveillance {
namespace sensor {

class SensorNode {
public:
    SensorNode(const std::string& node_id, 
               int node_index, 
               const config::AppConfig& cfg, 
               zmq::context_t& ctx);
    ~SensorNode() = default;

    void run_live();
    void run_deterministic();

    void stop();

private:
    void generate_events(double current_time_s);
    void send_status(double current_time_s);

    void emit_event();

    std::string node_id_;
    int node_index_;
    config::AppConfig cfg_;
    zmq::socket_t pub_socket_;

    std::mt19937_64 rng_;
    std::uniform_real_distribution<double> uniform_dist_{std::nextafter(0.0, 1.0), 1.0};
    
    // Stats and state
    uint64_t seq_num_{1};
    uint64_t start_time_ns_{0};
    double next_event_time_s_{0.0};
    double next_status_time_s_{0.0};
    std::atomic<bool> running_{true};
};

} // namespace sensor
} // namespace surveillance
