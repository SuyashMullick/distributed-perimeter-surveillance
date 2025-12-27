#pragma once

#include <string>
#include <uuid.h>

namespace surveillance {
namespace ids {

inline std::string generate_uuid() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    uuids::uuid_random_generator uuid_gen{gen};
    uuids::uuid const id = uuid_gen();
    return uuids::to_string(id);
}

} // namespace ids
} // namespace surveillance
