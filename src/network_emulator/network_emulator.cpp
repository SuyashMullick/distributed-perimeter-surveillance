#include "network_emulator.hpp"
#include "zmq_utils.hpp"
#include "time.hpp"
#include "logging.hpp"
#include "metrics.hpp"

#include <iostream>

namespace surveillance {
namespace network {

NetworkEmulator::NetworkEmulator(const config::AppConfig& cfg, zmq::context_t& ctx)
    : cfg_(cfg),
      sub_socket_(zmq_utils::create_subscriber(ctx, "tcp://127.0.0.1:7001", true)),
      pub_socket_(zmq_utils::create_publisher(ctx, "tcp://127.0.0.1:7002", true))
{
    rng_.seed(cfg_.network.network_seed);
}

NetworkEmulator::~NetworkEmulator() {
    stop();
}

void NetworkEmulator::stop() {
    running_ = false;
    if (incoming_thread_.joinable()) incoming_thread_.join();
    if (outgoing_thread_.joinable()) outgoing_thread_.join();
}

void NetworkEmulator::run() {
    incoming_thread_ = std::thread(&NetworkEmulator::process_incoming, this);
    if (cfg_.system.mode != "deterministic") {
        outgoing_thread_ = std::thread(&NetworkEmulator::process_outgoing, this);
    }
    
    // In deterministic mode, we process on the same thread or we rely on the main test loop.
    // However, since components are launched as independent processes and communicate via tcp,
    // "deterministic" means time is measured in ticks but the processes still run independently and
    // as fast as possible. Thus, deterministic logic on network emulator needs to compute delivery relative
    // to the originating monotonic elapsed time, or just forward it immediately with the same offset tracking.
    // The spec states: "It computes delivery time in simulated time and queues accordingly. Central processes queued deliveries in deterministic time order."
    // Let's implement queue and flush based on monotonic timestamps in the event.
    if (cfg_.system.mode == "deterministic") {
        outgoing_thread_ = std::thread(&NetworkEmulator::process_outgoing, this);
    }
}

void NetworkEmulator::process_incoming() {
    while (running_) {
        auto msg_opt = zmq_utils::receive_json(sub_socket_, true);
        if (!msg_opt) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        
        nlohmann::json msg = *msg_opt;
        metrics::increment("emulator.received_messages");

        // Loss logic (deterministic based on dropped p)
        if (cfg_.system.mode != "deterministic" && cfg_.network.loss_rate > 0.0) {
            if (uniform_dist_(rng_) < cfg_.network.loss_rate) {
                metrics::increment("emulator.dropped_messages");
                continue;
            }
        }
        
        // Latency and jitter
        int latency = cfg_.network.latency_ms;
        if (cfg_.system.mode != "deterministic" && cfg_.network.jitter_ms > 0) {
            std::uniform_int_distribution<int> jitter_dist(-cfg_.network.jitter_ms, cfg_.network.jitter_ms);
            latency += jitter_dist(rng_);
            if (latency < 0) latency = 0;
        }

        uint64_t source_time = time::monotonic_ns();
        if (cfg_.system.mode == "deterministic") {
            // In deterministic mode, source time is preserved as the actual monotonically sent time.
            // We use the event's embedded monotonic_ns to simulate ordering.
            source_time = msg["monotonic_ns"].get<uint64_t>();
        }
        
        uint64_t delivery_ns = source_time + (uint64_t(latency) * 1000000ULL);

        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            queue_.push({delivery_ns, msg});
        }
    }
}

void NetworkEmulator::process_outgoing() {
     while (running_) {
        std::vector<nlohmann::json> to_send;
        uint64_t current_time = time::monotonic_ns();
        
        // In deterministic mode we flush immediately anything that has a delivery time <= max queue item + simulated advancing time.
        // Actually, the spec clarifies that deterministic components run fast. "Network emulator processes messages in deterministic order and applies deterministic impairment without real sleeps: It computes delivery time in simulated time and queues accordingly."
        // We will pop items as soon as deterministic time allows. For simplicity, we can let central handle ordering or we sort here and flush.
        
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            while (!queue_.empty()) {
                if (cfg_.system.mode == "deterministic" || queue_.top().delivery_time_ns <= current_time) {
                    to_send.push_back(queue_.top().payload);
                    queue_.pop();
                } else {
                    break;
                }
            }
        }

        for (const auto& payload : to_send) {
            zmq_utils::publish_json(pub_socket_, payload);
            metrics::increment("emulator.forwarded_messages");
        }

        if (to_send.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

} // namespace network
} // namespace surveillance
