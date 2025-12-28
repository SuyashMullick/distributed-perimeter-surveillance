#pragma once

#include "config.hpp"
#include <zmq.hpp>
#include <nlohmann/json.hpp>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>
#include <random>

namespace surveillance {
namespace network {

struct QueuedMessage {
    uint64_t delivery_time_ns;
    nlohmann::json payload;
    
    bool operator>(const QueuedMessage& other) const {
        return delivery_time_ns > other.delivery_time_ns;
    }
};

class NetworkEmulator {
public:
    NetworkEmulator(const config::AppConfig& cfg, zmq::context_t& ctx);
    ~NetworkEmulator();

    void run();
    void stop();

private:
    void process_incoming();
    void process_outgoing();
    
    config::AppConfig cfg_;
    zmq::socket_t sub_socket_;
    zmq::socket_t pub_socket_;

    std::mt19937_64 rng_;
    std::uniform_real_distribution<double> uniform_dist_{0.0, 1.0};
    
    std::priority_queue<QueuedMessage, std::vector<QueuedMessage>, std::greater<QueuedMessage>> queue_;
    std::mutex queue_mutex_;

    std::atomic<bool> running_{true};
    std::thread incoming_thread_;
    std::thread outgoing_thread_;
};

} // namespace network
} // namespace surveillance
