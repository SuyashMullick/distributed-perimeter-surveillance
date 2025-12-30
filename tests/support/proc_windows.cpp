#include "proc.hpp"

#if defined(_WIN32)

#include <windows.h>
#include <stdexcept>

namespace surveillance {
namespace proc {

Process::Process(const std::string& cmd, const std::vector<std::string>& args) {
    std::string command_line = cmd;
    for (const auto& arg : args) {
        command_line += " " + arg;
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(NULL, const_cast<char*>(command_line.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        throw std::runtime_error("CreateProcess failed");
    }

    handle_ = pi.hProcess;
    CloseHandle(pi.hThread);
    running_ = true;
}

Process::~Process() {
    if (running_) {
        terminate();
        wait();
    }
}

void Process::terminate() {
    if (running_) {
        TerminateProcess(handle_, 0); // Graceful shutdown missing here but standard on windows tests
    }
}

void Process::wait() {
    if (running_) {
        WaitForSingleObject(handle_, INFINITE);
        CloseHandle(handle_);
        running_ = false;
    }
}

} // namespace proc
} // namespace surveillance

#endif
