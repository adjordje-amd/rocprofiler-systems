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
agent_manager::insert_agent(const rocprofiler_sdk::tool_agent& tool_agent, agent::device_type type,
                        size_t node_id, size_t process_id)
{
    std::cout << "Inserting agent with device handle: " << tool_agent.agent->id.handle
              << ", and agent id: " << _device_id
              << ", device type: " << (type == agent::device_type::gpu ? "GPU" : "CPU")
              << std::endl;

    const auto get_agent_type = [](const auto& type) {
        if(type != rocpd::agent::device_type::gpu &&
           type != rocpd::agent::device_type::cpu)
        {
            std::stringstream ss;
            ss << "Rocpd: insert agent info failed! Unknown agent type: "
               << static_cast<int>(type);
            throw std::runtime_error(ss.str());
        }
        return (type == rocpd::agent::device_type::gpu) ? "GPU" : "CPU";
    };

    auto _base_id = rocpd::data_processor::get_instance().insert_agent(
                        node_id, process_id, get_agent_type(type),
                        tool_agent.agent->node_id, tool_agent.agent->logical_node_id,
                        tool_agent.agent->logical_node_type_id, tool_agent.agent->device_id,
                        tool_agent.agent->name, tool_agent.agent->model_name,
                        tool_agent.agent->vendor_name, tool_agent.agent->product_name, "");

    agent _agent{ .handle = tool_agent.agent->id.handle, .device_id = _device_id++, .base_id = _base_id, .type = type };
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

const agent&
agent_manager::get_agent_by_handle(size_t device_handle) const
{
    ROCPROFSYS_VERBOSE(3, "Getting agent for device handle: %ld\n", device_handle);
    auto _agent = std::find_if(_agents.begin(), _agents.end(), [&](const auto& agent) {
        return agent.handle == device_handle;
    });
    if(_agent == _agents.end())
    {
        std::ostringstream oss;
        oss << "Agent not found for device handle: " << device_handle;
        throw std::out_of_range(oss.str());
    }
    return *_agent;
}

}  // namespace rocpd
}  // namespace rocprofsys
