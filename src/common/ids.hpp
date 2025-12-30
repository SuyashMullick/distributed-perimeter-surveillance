#pragma once

#include <string>
#include <uuid.h>

namespace surveillance {
namespace ids {

inline std::mt19937& get_gen() {
    static std::mt19937 gen{std::random_device{}()};
    return gen;
}

inline void seed(uint64_t s) {
    get_gen().seed(static_cast<uint32_t>(s));
}

inline std::string generate_uuid() {
    uuids::uuid_random_generator uuid_gen{get_gen()};
    return uuids::to_string(uuid_gen());
}

} // namespace ids
} // namespace surveillance
