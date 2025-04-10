#pragma once
#include <cstddef>
#include <vector>
#include <stdexcept>

#include "agent.hpp"
#include "library/rocprofiler-sdk/fwd.hpp"

namespace rocprofsys {
namespace rocpd {

struct agent_manager {
    static agent_manager& get_instance();

    agent_manager(const agent_manager&) = delete;
    agent_manager& operator=(const agent_manager&) = delete;
    agent_manager(agent_manager&&) = delete;
    agent_manager& operator=(agent_manager&&) = delete;
    ~agent_manager() = default;

    void insert_agent(agent _agent);

    const agent& get_agent(size_t device_id, agent::device_type type) const;

private:
    std::vector<agent> _agents;
    agent_manager() = default;
};

} // namespace rocpd
} // namespace rocprofsys