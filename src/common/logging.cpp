#include "logging.hpp"
#include "time.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/pattern_formatter.h>

#include <iostream>

namespace surveillance {
namespace logging {

class JsonFormatter : public spdlog::formatter {
    std::string component;
public:
    explicit JsonFormatter(const std::string& comp) : component(comp) {}
    
    void format(const spdlog::details::log_msg& msg, spdlog::memory_buf_t& dest) override {
        nlohmann::json j;
        j["timestamp_utc"] = time::utc_now_string();
        j["component"] = component;
        j["level"] = spdlog::level::to_string_view(msg.level).data();
        j["message"] = std::string(msg.payload.data(), msg.payload.size());
        
        // Note: the `fields` must be injected per log call.
        // We handle that via our wrapper functions which pre-format the message string or add to it.
        // For simplicity, we just dump the JSON payload if msg is already JSON.

        std::string text = std::string(msg.payload.data(), msg.payload.size());
        if (!text.empty() && text[0] == '{') {
            try {
                auto parsed = nlohmann::json::parse(text);
                j.merge_patch(parsed);
                j.erase("message"); // Move custom message field if any
            } catch (...) {
                // Ignore and treat as string
            }
        }
        
        std::string out = j.dump() + "\n";
        dest.append(out.data(), out.data() + out.size());
    }
    
    std::unique_ptr<spdlog::formatter> clone() const override {
        return std::make_unique<JsonFormatter>(component);
    }
};

static std::shared_ptr<spdlog::logger> logger;
static std::string g_component_name;
static int g_flush_every_n = 1;

void init(const std::string& component_name, const std::string& log_dir, int flush_every_n) {
    g_component_name = component_name;
    g_flush_every_n = flush_every_n;
    
    try {
        std::string log_file = log_dir + "/" + component_name + ".jsonl";
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file, true);
        auto console_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
        
        logger = std::make_shared<spdlog::logger>(component_name, spdlog::sinks_init_list{file_sink, console_sink});
        
        // Custom JSON formatter
        logger->set_formatter(std::make_unique<JsonFormatter>(component_name));
        
        if (flush_every_n <= 1) {
            logger->flush_on(spdlog::level::info);
        } // We will manually flush if needed
        
        spdlog::set_default_logger(logger);
    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Log init failed: " << ex.what() << std::endl;
        std::exit(1);
    }
}

void internal_log(spdlog::level::level_enum level, const std::string& msg, const nlohmann::json& fields) {
    if (!logger) return;
    
    nlohmann::json payload;
    payload["message"] = msg;
    if (!fields.empty()) {
        payload["fields"] = fields;
    } else {
        payload["fields"] = nlohmann::json::object();
    }
    logger->log(level, payload.dump());
    
    static int log_count = 0;
    if (++log_count >= g_flush_every_n) {
        logger->flush();
        log_count = 0;
    }
}

void info(const std::string& msg, const nlohmann::json& fields) {
    internal_log(spdlog::level::info, msg, fields);
}

void warn(const std::string& msg, const nlohmann::json& fields) {
    internal_log(spdlog::level::warn, msg, fields);
}

void error(const std::string& msg, const nlohmann::json& fields) {
    internal_log(spdlog::level::err, msg, fields);
}

void shutdown() {
    if (logger) {
        logger->flush();
    }
    spdlog::shutdown();
}

} // namespace logging
} // namespace surveillance
