#pragma once
#include <cstddef>
#include <stdexcept>
#include <vector>

#include "agent.hpp"
#include "library/rocprofiler-sdk/fwd.hpp"

namespace rocprofsys
{
namespace rocpd
{

struct agent_manager
{
    static agent_manager& get_instance();

    agent_manager(const agent_manager&)            = delete;
    agent_manager& operator=(const agent_manager&) = delete;
    agent_manager(agent_manager&&)                 = delete;
    agent_manager& operator=(agent_manager&&)      = delete;
    ~agent_manager()                               = default;

    void insert_agent(uint64_t device_handle, agent::device_type type);

    const agent& get_agent_by_id(size_t device_id, agent::device_type type) const;
    const agent& get_agent_by_handle(size_t device_id, agent::device_type type) const;

private:
    std::vector<agent> _agents;
    size_t             _device_id{ 0 };
    agent_manager() = default;
};

}  // namespace rocpd
}  // namespace rocprofsys
