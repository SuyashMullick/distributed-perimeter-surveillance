#include "sensor_node.hpp"
#include "ids.hpp"
#include "time.hpp"
#include "zmq_utils.hpp"
#include "logging.hpp"

#include <cmath>
#include <thread>
#include <iostream>

namespace surveillance {
namespace sensor {

SensorNode::SensorNode(const std::string& node_id, int node_index, const config::AppConfig& cfg, zmq::context_t& ctx)
    : node_id_(node_id), node_index_(node_index), cfg_(cfg),
      pub_socket_(zmq_utils::create_publisher(ctx, "tcp://127.0.0.1:7001", false))
{
    rng_.seed(cfg_.system.seed_base + node_index_);
    next_event_time_s_ = -std::log(uniform_dist_(rng_)) / cfg_.sensor.event_rate_hz;
    next_status_time_s_ = 1.0 / cfg_.sensor.status_rate_hz;
    start_time_ns_ = time::monotonic_ns();
}

void SensorNode::stop() {
    running_ = false;
}

void SensorNode::emit_event(double current_time_s) {
    // Determine type
    double p = uniform_dist_(rng_);
    std::string event_type;
    double amp_mean, amp_std, en_mean, en_std;

    if (p <= 0.50) {
        event_type = "WALKING";
        amp_mean = 0.4; amp_std = 0.1;
        en_mean = 10.0; en_std = 3.0;
    } else if (p <= 0.70) {
        event_type = "VEHICLE";
        amp_mean = 0.7; amp_std = 0.1;
        en_mean = 25.0; en_std = 5.0;
    } else if (p <= 0.80) {
        event_type = "DIGGING";
        amp_mean = 0.8; amp_std = 0.15;
        en_mean = 30.0; en_std = 8.0;
    } else {
        event_type = "WIND";
        amp_mean = 0.2; amp_std = 0.08;
        en_mean = 6.0; en_std = 2.0;
    }

    std::normal_distribution<double> dist_amp(amp_mean, amp_std);
    std::normal_distribution<double> dist_en(en_mean, en_std);

    double signal_amplitude = std::max(0.0, dist_amp(rng_));
    double signal_energy = std::max(0.0, dist_en(rng_));
    uint32_t generated_seed = rng_();

    uint64_t mono_ns = time::monotonic_ns();
    std::string utc_str = time::utc_now_string();
    if (cfg_.system.mode == "deterministic") {
        mono_ns = (uint64_t)(current_time_s * 1e9);
        utc_str = time::format_utc_ms((uint64_t)(current_time_s * 1000.0) + 1700000000000ULL); // Baseline arbitrary deterministic epoch + time
    }

    nlohmann::json msg = {
        {"msg_type", "DisturbanceEvent"},
        {"event_id", ids::generate_uuid()},
        {"node_id", node_id_},
        {"sequence_number", seq_num_++},
        {"timestamp_utc", utc_str},
        {"monotonic_ns", mono_ns},
        {"signal_amplitude", signal_amplitude},
        {"signal_energy", signal_energy},
        {"event_type", event_type},
        {"generated_seed", generated_seed}
    };

    zmq_utils::publish_json(pub_socket_, msg);
    
    // log locally as well to support TC-FT-001 mapping
    logging::info("Generated event", msg);
}

void SensorNode::send_status(double current_time_s) {
    uint64_t mono_ns = time::monotonic_ns();
    std::string utc_str = time::utc_now_string();
    if (cfg_.system.mode == "deterministic") {
        mono_ns = (uint64_t)(current_time_s * 1e9);
        utc_str = time::format_utc_ms((uint64_t)(current_time_s * 1000.0) + 1700000000000ULL);
    }
    nlohmann::json msg = {
        {"msg_type", "NodeStatus"},
        {"node_id", node_id_},
        {"timestamp_utc", utc_str},
        {"monotonic_ns", mono_ns},
        {"health", "OK"},
        {"uptime_s", current_time_s},
        {"last_sequence_number", seq_num_ - 1}
    };
    zmq_utils::publish_json(pub_socket_, msg);
}

void SensorNode::generate_events(double current_time_s) {
    while (next_event_time_s_ <= current_time_s) {
        emit_event(current_time_s);
        next_event_time_s_ += -std::log(uniform_dist_(rng_)) / cfg_.sensor.event_rate_hz;
    }

    if (next_status_time_s_ <= current_time_s) {
        send_status(current_time_s);
        next_status_time_s_ += 1.0 / cfg_.sensor.status_rate_hz;
    }
}

void SensorNode::run_live() {
    double start_time_s = time::monotonic_ns() / 1e9;
    while (running_) {
        double current_time_s = (time::monotonic_ns() / 1e9) - start_time_s;
        generate_events(current_time_s);

        if (current_time_s >= cfg_.system.duration_s) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void SensorNode::run_deterministic() {
    double tick_s = 0.01; // 100 Hz
    int ticks = cfg_.system.duration_s / tick_s;
    
    for (int i = 0; i < ticks && running_; ++i) {
        double current_time_s = i * tick_s;
        generate_events(current_time_s);
    }
}

} // namespace sensor
} // namespace surveillance
