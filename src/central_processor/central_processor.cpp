#include "central_processor.hpp"
#include "zmq_utils.hpp"
#include "time.hpp"
#include "ids.hpp"
#include "logging.hpp"
#include "metrics.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/pattern_formatter.h>

namespace surveillance {
namespace central {

class AlertFormatter : public spdlog::formatter {
public:
    void format(const spdlog::details::log_msg& msg, spdlog::memory_buf_t& dest) override {
        std::string text = std::string(msg.payload.data(), msg.payload.size());
        std::string out = text + "\n";
        dest.append(out.data(), out.data() + out.size());
    }
    std::unique_ptr<spdlog::formatter> clone() const override {
        return std::make_unique<AlertFormatter>();
    }
};

CentralProcessor::CentralProcessor(const config::AppConfig& cfg, zmq::context_t& ctx)
    : cfg_(cfg),
      sub_socket_(zmq_utils::create_subscriber(ctx, "tcp://127.0.0.1:7002", false))
{
    // Setup pure alerts.jsonl logger
    std::string alert_file = cfg_.logging.log_dir + "/alerts.jsonl";
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(alert_file, true);
    alerts_logger_ = std::make_shared<spdlog::logger>("alerts", file_sink);
    alerts_logger_->set_formatter(std::make_unique<AlertFormatter>());
    alerts_logger_->flush_on(spdlog::level::info);
}

CentralProcessor::~CentralProcessor() {
    stop();
}

void CentralProcessor::stop() {
    running_ = false;
    if (processing_thread_.joinable()) processing_thread_.join();
    if (state_writer_thread_.joinable()) state_writer_thread_.join();
    
    if (alerts_logger_) {
        alerts_logger_->flush();
    }
}

void CentralProcessor::run() {
    processing_thread_ = std::thread(&CentralProcessor::process_messages, this);
    state_writer_thread_ = std::thread(&CentralProcessor::write_state_loop, this);
}

uint64_t CentralProcessor::parse_utc_to_ms(const std::string& utc_iso) {
    // Basic approximate parsing for exact millisecond extraction assuming fixed width format "%Y-%m-%dT%H:%M:%S.123Z"
    if (utc_iso.size() >= 24) {
        std::tm tm{};
        int ms = 0;
        sscanf(utc_iso.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d.%3dZ",
               &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
               &tm.tm_hour, &tm.tm_min, &tm.tm_sec, &ms);
        tm.tm_year -= 1900;
        tm.tm_mon -= 1;
        
#if defined(_WIN32)
        time_t t = _mkgmtime(&tm);
#else
        time_t t = timegm(&tm);
#endif
        return (static_cast<uint64_t>(t) * 1000ULL) + ms;
    }
    return 0; // Fallback
}

void CentralProcessor::handle_event(const nlohmann::json& msg) {
    double amp = msg.value("signal_amplitude", 0.0);
    double en = msg.value("signal_energy", 0.0);
    std::string type = msg.value("event_type", "");

    std::string classification = "LOW";
    if (type == "DIGGING" || (en >= 22.0 && amp >= 0.65)) {
        classification = "HIGH";
    } else if (type == "VEHICLE" || (en >= 14.0 && amp >= 0.45)) {
        classification = "MEDIUM";
    }

    uint64_t central_utc_ms = time::utc_now_ms();
    uint64_t event_utc_ms = parse_utc_to_ms(msg.value("timestamp_utc", ""));
    uint64_t mono_ns = time::monotonic_ns();
    std::string timestamp_utc = time::utc_now_string();

    if (cfg_.system.mode == "deterministic") {
        central_utc_ms = event_utc_ms + cfg_.network.latency_ms + 1; 
        timestamp_utc = time::format_utc_ms(central_utc_ms);
        mono_ns = msg.value("monotonic_ns", 0ULL) + (cfg_.network.latency_ms + 1) * 1000000ULL;
    }
    double latency = std::max(0.0, static_cast<double>(central_utc_ms) - static_cast<double>(event_utc_ms));

    nlohmann::json alert = {
        {"msg_type", "CentralAlert"},
        {"alert_id", ids::generate_uuid()},
        {"event_id", msg["event_id"]},
        {"source_node_id", msg["node_id"]},
        {"timestamp_utc", timestamp_utc},
        {"monotonic_ns", mono_ns},
        {"classification", classification},
        {"processing_latency_ms", latency}
    };

    alerts_logger_->info(alert.dump());
    
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        recent_alerts_.push_front(alert);
        if (recent_alerts_.size() > static_cast<size_t>(cfg_.central.alerts_buffer)) {
            recent_alerts_.pop_back();
        }
    }

    metrics::increment("central.alerts_generated");
}

void CentralProcessor::handle_status(const nlohmann::json& msg) {
    std::string node_id = msg.value("node_id", "");
    std::lock_guard<std::mutex> lock(state_mutex_);
    auto& state = nodes_[node_id];
    state.health = msg.value("health", "UNKNOWN");
    state.uptime_s = msg.value("uptime_s", 0.0);
    state.last_sequence_number = msg.value("last_sequence_number", 0ULL);
    state.last_seen_utc_ms = time::utc_now_ms();
}

void CentralProcessor::process_messages() {
    while (running_) {
        auto msg_opt = zmq_utils::receive_json(sub_socket_, false);
        if (!msg_opt) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        nlohmann::json msg = *msg_opt;
        std::string msg_type = msg.value("msg_type", "");

        if (msg_type == "DisturbanceEvent") {
            handle_event(msg);
        } else if (msg_type == "NodeStatus") {
            handle_status(msg);
        }
    }
}

void CentralProcessor::write_state_loop() {
    std::string state_file = cfg_.logging.log_dir + "/central_state.json";
    std::string temp_file = state_file + ".tmp";

    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        nlohmann::json state_json;
        uint64_t now_ms = time::utc_now_ms();

        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            
            nlohmann::json nodes_json = nlohmann::json::object();
            for (auto& [node_id, state] : nodes_) {
                double age_s = (now_ms - state.last_seen_utc_ms) / 1000.0;
                if (age_s > cfg_.central.heartbeat_timeout_s) {
                    state.health = "FAILED";
                }
                nodes_json[node_id] = {
                    {"health", state.health},
                    {"uptime_s", state.uptime_s},
                    {"last_seen_age_s", age_s},
                    {"last_sequence_number", state.last_sequence_number}
                };
            }

            state_json["nodes"] = nodes_json;
            state_json["metrics"] = metrics::get_all();
            state_json["recent_alerts"] = recent_alerts_;
        }

        try {
            std::ofstream f(temp_file);
            f << state_json.dump() << "\n";
            f.close();
            std::filesystem::rename(temp_file, state_file);
        } catch (...) {
            logging::error("Failed to write central_state.json atomicity");
        }
    }
}

} // namespace central
} // namespace surveillance
