#pragma once

#include <string>
#include <cstdint>

namespace surveillance {
namespace time {

// Monotonic nanoseconds for duration/determinism tracking
uint64_t monotonic_ns();

// ISO-8601 UTC string (e.g. "2026-02-23T12:34:56.123Z")
std::string utc_now_string();

// UTC milliseconds since unix epoch, needed for central latency processing
uint64_t utc_now_ms();

std::string format_utc_ms(uint64_t ms);

} // namespace time
} // namespace surveillance
