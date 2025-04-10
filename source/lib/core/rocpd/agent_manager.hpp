#pragma once
#include <cstddef>
#include <vector>
#include <stdexcept>

#include "library/rocprofiler-sdk/fwd.hpp"

using tool_agent_t = rocprofsys::rocprofiler_sdk::tool_agent;


struct agent_manager {
    static agent_manager& get_instance();

    agent_manager(const agent_manager&) = delete;
    agent_manager& operator=(const agent_manager&) = delete;
    agent_manager(agent_manager&&) = delete;
    agent_manager& operator=(agent_manager&&) = delete;
    ~agent_manager() = default;

    void insert_agent(tool_agent_t agent);

    const rocprofiler_agent_v0_t* get_gpu_agent(size_t id) const;

    const rocprofiler_agent_v0_t* get_cpu_agent(size_t id) const;

private:
    std::vector<tool_agent_t> _agents;
    agent_manager() = default;
};