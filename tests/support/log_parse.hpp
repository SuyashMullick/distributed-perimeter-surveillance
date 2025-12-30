#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace surveillance {
namespace log_parse {

std::vector<nlohmann::json> read_jsonl(const std::string& filepath);

} // namespace log_parse
} // namespace surveillance
