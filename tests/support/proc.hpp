#pragma once
#include <string>
#include <vector>

namespace surveillance {
namespace proc {

typedef void* handle_t; // Abstract handle

class Process {
public:
    Process(const std::string& cmd, const std::vector<std::string>& args);
    ~Process();

    void wait();
    void terminate();

private:
    handle_t handle_;
    bool running_;
};

} // namespace proc
} // namespace surveillance
