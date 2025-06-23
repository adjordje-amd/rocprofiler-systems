#include "agent_manager.hpp"
#include "core/rocpd/data_processor.hpp"
#include "debug.hpp"

namespace rocprofsys
{
namespace rocpd
{

agent_manager&
agent_manager::get_instance()
{
    static agent_manager instance;
    return instance;
}

void
agent_manager::insert_agent(const rocprofiler_agent_v0_t* _agent)
{
    ROCPROFSYS_VERBOSE(3, "Inserting agent with device handle: %d, and agent id: %ld, device type: %s",
        _agent->id.handle,
        (_agent->type == ROCPROFILER_AGENT_TYPE_GPU ? _gpu_agents_cnt : _cpu_agents_cnt),
        (_agent->type == ROCPROFILER_AGENT_TYPE_GPU ? "GPU" : "CPU"));

    _agents.emplace_back(std::make_shared<agent>(agent{
        _agent,
        (_agent->type == ROCPROFILER_AGENT_TYPE_GPU ? _gpu_agents_cnt++ : _cpu_agents_cnt++),
    }));
}

const agent&
agent_manager::get_agent_by_id(size_t device_id, rocprofiler_agent_type_t type) const
{
    ROCPROFSYS_VERBOSE(3, "Getting agent for device id: %ld, type %s\n", device_id,
                       (type == ROCPROFILER_AGENT_TYPE_GPU) ? "GPU" : "CPU");
    auto _agent = std::find_if(_agents.begin(), _agents.end(), [&](const auto& agent_ptr) {
        return agent_ptr->agent->type == type && agent_ptr->device_id == device_id;
    });
    if(_agent == _agents.end())
    {
        std::ostringstream oss;
        oss << "Agent not found for device id: " << device_id
            << ", type: " << (type == ROCPROFILER_AGENT_TYPE_GPU ? "GPU" : "CPU");
        throw std::out_of_range(oss.str());
    }
    return **_agent;
}

const agent&
agent_manager::get_agent_by_handle(uint64_t device_handle, rocprofiler_agent_type_t type) const
{
    ROCPROFSYS_VERBOSE(3, "Getting agent for device handle: %ld, type %s\n",
                       device_handle, (type == ROCPROFILER_AGENT_TYPE_GPU ? "GPU" : "CPU"));
    auto _agent = std::find_if(_agents.begin(), _agents.end(), [&](const auto& agent_ptr) {
        return agent_ptr->agent->type == type && agent_ptr->agent->id.handle == device_handle;
    });
    if(_agent == _agents.end())
    {
        std::ostringstream oss;
        oss << "Agent not found for device handle: " << device_handle
            << ", type: " << (type == ROCPROFILER_AGENT_TYPE_GPU ? "GPU" : "CPU");
        throw std::out_of_range(oss.str());
    }
    return **_agent;
}

const agent&
agent_manager::get_agent_by_handle(size_t device_handle) const
{
    ROCPROFSYS_VERBOSE(3, "Getting agent for device handle: %ld\n", device_handle);
    auto _agent = std::find_if(_agents.begin(), _agents.end(), [&](const auto& agent_ptr) {
        return agent_ptr->agent->id.handle == device_handle;
    });
    if(_agent == _agents.end())
    {
        std::ostringstream oss;
        oss << "Agent not found for device handle: " << device_handle;
        throw std::out_of_range(oss.str());
    }
    return **_agent;
}

std::vector<std::shared_ptr<agent>>
agent_manager::get_agents_by_type(rocprofiler_agent_type_t type)
{
    ROCPROFSYS_VERBOSE(3, "Getting agent for device type: %s\n",
        type == ROCPROFILER_AGENT_TYPE_GPU ? "GPU" : "CPU");

    std::vector<std::shared_ptr<agent>> agents;
    for(const auto& agent_ptr : _agents)
    {
        if(agent_ptr->agent->type == type)
        {
            agents.push_back(agent_ptr);
        }
    }
    if ( agents.empty()) {
        ROCPROFSYS_THROW("No %s agents found!", (type == ROCPROFILER_AGENT_TYPE_GPU ? "GPU" : "CPU"));
    }
    return agents;
}

std::vector<std::shared_ptr<agent>>
agent_manager::get_agents()
{
    return _agents;
}

size_t
agent_manager::get_gpu_agents_count() { return _gpu_agents_cnt; }

size_t
agent_manager::get_cpu_agents_count() { return _cpu_agents_cnt; }

}  // namespace rocpd
}  // namespace rocprofsys
