#include "agent_manager.hpp"

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
agent_manager::insert_agent(uint64_t device_handle, agent::device_type type)
{
    std::cout << "Inserting agent with device handle: " << device_handle
              << ", and agent id: " << _device_id
              << ", device type: " << (type == agent::device_type::gpu ? "GPU" : "CPU")
              << std::endl;

    agent _agent{ .handle = device_handle, .device_id = _device_id++, .type = type };
    _agents.emplace_back(_agent);
}

const agent&
agent_manager::get_agent_by_id(size_t device_id, agent::device_type type) const
{
    ROCPROFSYS_VERBOSE(3, "Getting agent for device id: %ld, type %s\n", device_id,
                       (type == agent::device_type::gpu ? "GPU" : "CPU"));
    auto _agent = std::find_if(_agents.begin(), _agents.end(), [&](const auto& agent) {
        return agent.type == type && agent.device_id == device_id;
    });
    if(_agent == _agents.end())
    {
        std::ostringstream oss;
        oss << "Agent not found for device id: " << device_id
            << ", type: " << (type == rocpd::agent::device_type::gpu ? "GPU" : "CPU");
        throw std::out_of_range(oss.str());
    }
    return *_agent;
}

const agent&
agent_manager::get_agent_by_handle(uint64_t device_handle, agent::device_type type) const
{
    ROCPROFSYS_VERBOSE(3, "Getting agent for device handle: %ld, type %s\n",
                       device_handle, (type == agent::device_type::gpu ? "GPU" : "CPU"));
    auto _agent = std::find_if(_agents.begin(), _agents.end(), [&](const auto& agent) {
        return agent.type == type && agent.handle == device_handle;
    });
    if(_agent == _agents.end())
    {
        std::ostringstream oss;
        oss << "Agent not found for device handle: " << device_handle
            << ", type: " << (type == rocpd::agent::device_type::gpu ? "GPU" : "CPU");
        throw std::out_of_range(oss.str());
    }
    return *_agent;
}

}  // namespace rocpd
}  // namespace rocprofsys
