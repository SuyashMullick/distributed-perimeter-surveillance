#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <stdexcept>

namespace surveillance {
namespace json_utils {

inline bool has_fields(const nlohmann::json& j, const std::vector<std::string>& fields) {
    if (!j.is_object()) return false;
    for (const auto& f : fields) {
        if (!j.contains(f)) return false;
    }
    return true;
}

inline void require_fields(const nlohmann::json& j, const std::vector<std::string>& fields) {
    if (!has_fields(j, fields)) {
        throw std::runtime_error("Missing required JSON fields");
    }
}

} // namespace json_utils
} // namespace surveillance
