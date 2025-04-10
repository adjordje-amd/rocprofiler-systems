#pragma once
#include <cstddef>
#include <vector>
#include <stdexcept>

#include "library/rocprofiler-sdk/fwd.hpp"

using tool_agent_t = rocprofsys::rocprofiler_sdk::tool_agent;


struct agent_manager {
    static agent_manager& get_instance() {
        static agent_manager instance;
        return instance;
    }

    agent_manager(const agent_manager&) = delete;
    agent_manager& operator=(const agent_manager&) = delete;
    agent_manager(agent_manager&&) = delete;
    agent_manager& operator=(agent_manager&&) = delete;
    ~agent_manager() = default;

    void insert_agent(tool_agent_t agent) {
        _agents.push_back(agent);
    }

    auto get_agent(size_t gpu_id) const {
        auto it = std::find_if(_agents.begin(), _agents.end(),
            [gpu_id](const tool_agent_t& agent) { return agent.device_id == gpu_id; });
        if (it == _agents.end()) {
            throw std::out_of_range("GPU ID not found");
        }

        return it->agent;
    }

private:
    std::vector<tool_agent_t> _agents;
    agent_manager() = default;
};