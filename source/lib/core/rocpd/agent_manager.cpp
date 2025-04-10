#include "agent_manager.hpp"

#include "debug.hpp"

agent_manager& agent_manager::get_instance() {
    static agent_manager instance;
    return instance;
}

void agent_manager::insert_agent(tool_agent_t agent) {
    std::cout << "Inserting agent with device id: " << agent.device_id << ", and agent id: " << agent.agent->id.handle << ", device type: " << agent.agent->type << std::endl;
    _agents.push_back(agent);
}

const rocprofiler_agent_v0_t* agent_manager::get_gpu_agent(size_t id) const {
    ROCPROFSYS_VERBOSE(3, "Getting GPU agent for device id: %ld\n", id);
    auto it = std::find_if(_agents.begin(), _agents.end(),
        [id](const auto& agent) {return agent.agent->type == ROCPROFILER_AGENT_TYPE_GPU && agent.device_id == id;});
    if (it == _agents.end()) {
        std::ostringstream oss;
        for(auto x : _agents) {
            std::cout << "Agent id: " << x.agent->id.handle << ", device id: " << x.device_id << ", device type: " << x.agent->type << std::endl;
        }
        oss << "GPU Agent not found for device id: " << id;
        throw std::out_of_range(oss.str());
    }
    return it->agent;
}

const rocprofiler_agent_v0_t* agent_manager::get_cpu_agent(size_t id) const {
    auto it = std::find_if(_agents.begin(), _agents.end(),
        [id](const auto& agent) { return agent.agent->type == ROCPROFILER_AGENT_TYPE_CPU && agent.device_id == id; });
    if (it == _agents.end()) {
        std::ostringstream oss;
        oss << "CPU Agent not found for device id: " << id;
        throw std::out_of_range(oss.str());
    }
    return it->agent;
}