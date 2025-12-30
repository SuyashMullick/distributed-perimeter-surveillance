#include "time.hpp"
#include <chrono>
#include <sstream>
#include <iomanip>

namespace surveillance {
namespace time {

uint64_t monotonic_ns() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
}

std::string utc_now_string() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now{};
#if defined(_WIN32)
    gmtime_s(&tm_now, &time_t_now);
#else
    gmtime_r(&time_t_now, &tm_now);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%dT%H:%M:%S.");
    oss << std::setfill('0') << std::setw(3) << ms.count() << "Z";
    return oss.str();
}

uint64_t utc_now_ms() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

std::string format_utc_ms(uint64_t ms) {
    auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(ms));
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm* tm = std::gmtime(&t);
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%dT%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << (ms % 1000) << "Z";
    return oss.str();
}

} // namespace time
} // namespace surveillance
