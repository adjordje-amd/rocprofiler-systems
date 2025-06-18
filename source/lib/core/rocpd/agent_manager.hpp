#pragma once
#include <cstddef>
#include <stdexcept>
#include <vector>
#include <memory>

#include "agent.hpp"

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

    void insert_agent(const rocprofiler_agent_v0_t* agent,
                        size_t node_id, size_t process_id);

    const agent& get_agent_by_id(size_t device_id, rocprofiler_agent_type_t type) const;
    const agent& get_agent_by_handle(size_t device_id, rocprofiler_agent_type_t type) const;
    const agent& get_agent_by_handle(size_t device_handle) const;

    std::vector<std::shared_ptr<agent>> get_agents_by_type(rocprofiler_agent_type_t type);
    std::vector<std::shared_ptr<agent>> get_agents();

    size_t get_gpu_agents_count();
    size_t get_cpu_agents_count();

private:
    std::vector<std::shared_ptr<agent>> _agents;
    size_t                              _gpu_agents_cnt { 0 };
    size_t                              _cpu_agents_cnt { 0 };
    agent_manager() = default;
};

}  // namespace rocpd
}  // namespace rocprofsys
