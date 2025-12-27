#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace surveillance {
namespace logging {

void init(const std::string& component_name, const std::string& log_dir, int flush_every_n);

void info(const std::string& msg, const nlohmann::json& fields = nlohmann::json::object());
void warn(const std::string& msg, const nlohmann::json& fields = nlohmann::json::object());
void error(const std::string& msg, const nlohmann::json& fields = nlohmann::json::object());

void shutdown();

} // namespace logging
} // namespace surveillance
