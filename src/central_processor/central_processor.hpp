#pragma once
#include "config.hpp"
#include <zmq.hpp>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <deque>
#include <nlohmann/json.hpp>
#include <spdlog/logger.h>

namespace surveillance {
namespace central {

struct NodeState {
    std::string health{"UNKNOWN"};
    double uptime_s{0.0};
    uint64_t last_sequence_number{0};
    uint64_t last_seen_utc_ms{0};
};

class CentralProcessor {
public:
    CentralProcessor(const config::AppConfig& cfg, zmq::context_t& ctx);
    ~CentralProcessor();

    void run();
    void stop();

private:
    void process_messages();
    void write_state_loop();

    void handle_event(const nlohmann::json& msg);
    void handle_status(const nlohmann::json& msg);

    uint64_t parse_utc_to_ms(const std::string& utc_iso);

    config::AppConfig cfg_;
    zmq::socket_t sub_socket_;
    
    std::atomic<bool> running_{true};
    std::thread processing_thread_;
    std::thread state_writer_thread_;

    std::unordered_map<std::string, NodeState> nodes_;
    std::deque<nlohmann::json> recent_alerts_;
    std::mutex state_mutex_;

    std::shared_ptr<spdlog::logger> alerts_logger_;
};

} // namespace central
} // namespace surveillance
