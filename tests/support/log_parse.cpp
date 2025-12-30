#include "log_parse.hpp"
#include <fstream>
#include <iostream>

namespace surveillance {
namespace log_parse {

std::vector<nlohmann::json> read_jsonl(const std::string& filepath) {
    std::vector<nlohmann::json> result;
    std::ifstream f(filepath);
    if (!f.is_open()) return result;

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        try {
            result.push_back(nlohmann::json::parse(line));
        } catch (...) {
            // Drop invalid line
        }
    }
    return result;
}

} // namespace log_parse
} // namespace surveillance
