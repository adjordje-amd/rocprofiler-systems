#pragma once
#include <cstdint>
#include <unistd.h>

struct process_info
{
    uint64_t pid;
    uint64_t ppid;
    uint64_t tid;
    uint64_t start_time;
    uint64_t end_time;
};

inline process_info
get_process_info()
{
    process_info info;
    info.pid  = static_cast<uint64_t>(getpid());
    info.ppid = static_cast<uint64_t>(getppid());
    info.tid  = static_cast<uint64_t>(gettid());
    return info;
}
