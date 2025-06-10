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
agent_manager::insert_agent(agent _agent)
{
    std::cout << "Inserting agent with device id: " << _agent.device_id
              << ", and agent id: " << _agent.id << ", device type: "
              << (_agent.type == agent::device_type::gpu ? "GPU" : "CPU") << std::endl;
    _agents.push_back(_agent);
}

const agent&
agent_manager::get_agent(size_t device_id, agent::device_type type) const
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

}  // namespace rocpd
}  // namespace rocprofsys
