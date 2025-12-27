#include "metrics.hpp"

namespace surveillance {
namespace metrics {

static std::unordered_map<std::string, std::atomic<uint64_t>> g_metrics;
static std::shared_mutex g_mutex;

void increment(const std::string& key) {
    std::shared_lock<std::shared_mutex> lock(g_mutex);
    auto it = g_metrics.find(key);
    if (it != g_metrics.end()) {
        it->second++;
    } else {
        lock.unlock();
        std::unique_lock<std::shared_mutex> ulock(g_mutex);
        g_metrics[key]++;
    }
}

void add(const std::string& key, uint64_t value) {
    std::shared_lock<std::shared_mutex> lock(g_mutex);
    auto it = g_metrics.find(key);
    if (it != g_metrics.end()) {
        it->second += value;
    } else {
        lock.unlock();
        std::unique_lock<std::shared_mutex> ulock(g_mutex);
        g_metrics[key] += value;
    }
}

uint64_t get(const std::string& key) {
    std::shared_lock<std::shared_mutex> lock(g_mutex);
    auto it = g_metrics.find(key);
    if (it != g_metrics.end()) {
        return it->second.load();
    }
    return 0;
}

std::unordered_map<std::string, uint64_t> get_all() {
    std::unordered_map<std::string, uint64_t> result;
    std::shared_lock<std::shared_mutex> lock(g_mutex);
    for (const auto& [k, v] : g_metrics) {
        result[k] = v.load();
    }
    return result;
}

} // namespace metrics
} // namespace surveillance
