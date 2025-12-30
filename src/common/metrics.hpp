#pragma once
#include <string>
#include <atomic>
#include <unordered_map>
#include <shared_mutex>
#include <mutex>

namespace surveillance {
namespace metrics {

void increment(const std::string& key);
uint64_t get(const std::string& key);
void add(const std::string& key, uint64_t value);
std::unordered_map<std::string, uint64_t> get_all();

} // namespace metrics
} // namespace surveillance
