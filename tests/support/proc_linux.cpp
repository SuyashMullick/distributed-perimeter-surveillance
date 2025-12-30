#include "proc.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <spawn.h>
#include <stdexcept>
#include <iostream>

extern char **environ;

namespace surveillance {
namespace proc {

Process::Process(const std::string& cmd, const std::vector<std::string>& args) {
    std::vector<char*> c_args;
    c_args.push_back(const_cast<char*>(cmd.c_str()));
    for (const auto& a : args) {
        c_args.push_back(const_cast<char*>(a.c_str()));
    }
    c_args.push_back(nullptr);

    pid_t pid;
    int status = posix_spawn(&pid, cmd.c_str(), nullptr, nullptr, c_args.data(), environ);
    if (status == 0) {
        handle_ = reinterpret_cast<handle_t>(static_cast<intptr_t>(pid));
        running_ = true;
    } else {
        throw std::runtime_error("posix_spawn failed for " + cmd);
    }
}

Process::~Process() {
    if (running_) {
        terminate();
        wait();
    }
}

void Process::terminate() {
    if (running_) {
        pid_t pid = static_cast<pid_t>(reinterpret_cast<intptr_t>(handle_));
        kill(pid, SIGTERM);
        // give it a bit of time, then SIGKILL if we wanted strict, but we assume SIGTERM works for clean shutdown tests
    }
}

void Process::wait() {
    if (running_) {
        pid_t pid = static_cast<pid_t>(reinterpret_cast<intptr_t>(handle_));
        int status;
        waitpid(pid, &status, 0);
        running_ = false;
    }
}

} // namespace proc
} // namespace surveillance
