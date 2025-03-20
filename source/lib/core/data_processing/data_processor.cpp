#include "data_processor.hpp"

namespace rocprofsys{
namespace {
    
static inline constexpr const char* 
get_agent_type(data_processor::agent_type agent_type) {
    switch (agent_type)
    {
        case data_processor::agent_type::cpu:
            return "CPU";
        
        case data_processor::agent_type::gpu:
            return "GPU";
        
        default: 
            return "UNKNOWN";
    }
}

}

constexpr char* CATEHORY_NAME_SMI_DEVICE_BUSY = "Device Busy";
constexpr char* CATEHORY_NAME_SMI_DEVICE_MEMORY_USAGE = "Device Memory Usage";
constexpr char* CATEHORY_NAME_SMI_DEVICE_POWER = "Device Power";
constexpr char* CATEHORY_NAME_SMI_DEVICE_TEMPERATURE = "Device Temperature";

data_processor::data_processor() {
    data_storage::database::get_instance().initialize_schema();

    auto position = create_string(CATEHORY_NAME_SMI_DEVICE_BUSY);
    _category_map[category_id::smi_device_busy] = position;

    position = create_string(CATEHORY_NAME_SMI_DEVICE_MEMORY_USAGE);
    _category_map[category_id::smi_device_memory_usage] = position;

    position = create_string(CATEHORY_NAME_SMI_DEVICE_POWER);
    _category_map[category_id::smi_device_power] = position;

    position = create_string(CATEHORY_NAME_SMI_DEVICE_TEMPERATURE);
    _category_map[category_id::smi_device_temperature] = position;
}

data_processor& data_processor::get_instance(){
    static data_processor _instance;
    return _instance;
}

int data_processor::create_string(std::string_view str){
    data_storage::queries::table_insert_query query;
    data_storage::database::get_instance()
                            .execute_query(
                                query.set_table_name("rocpd_string")
                                    .set_columns("id", "string")
                                    .set_values(_string_id, str)
                                    .get_query_string());
    
    return _string_id++;
}

void 
data_processor::create_agent(const data_processor::agent_descriptor& agent) {
    data_storage::queries::table_insert_query query;
    data_storage::database::get_instance()
                            .execute_query(
                                query.set_table_name("rocpd_agent")
                                    .set_columns("id", "node_id", "type", "absolute_index", "logical_index", "type_index", "uuid", "name", 
                                                    "model_name", "vendor_name", "product_name", "user_name", "extdata")
                                    .set_values(agent.id, agent.node_id, get_agent_type(agent.type), agent.absolute_index, agent.logical_index, agent.type_index, 
                                                agent.uuid, agent.name, agent.model_name, agent.vendor_name, agent.product_name, agent.user_name, agent.extdata)
                                    .get_query_string());
}

void 
data_processor::create_track(std::string_view track_name, uint32_t node_id, pid_t pid, std::thread::id tid){
    if (_track_name_map.find(track_name) != _track_name_map.end()) {
        // TODO: Add warning message
        printf("Track already exists!");
        return;
    }

    auto name_id = create_string(track_name);
    _track_name_map[track_name] = _track_id;
    data_storage::queries::table_insert_query query;
    data_storage::database::get_instance()
                            .execute_query(
                                query.set_table_name("_rocpd_track")
                                    .set_columns("id", "node_id", "pid", "tid", "name_id")
                                    .set_values(_track_id++, node_id, pid, tid, name_id)
                                    .get_query_string());
}                            

void 
data_processor::add_sample(std::string_view track_name, uint64_t timestamp) {
    // if (_track_name_map.find(track_name) == _track_name_map.end()) {
    //     // TODO: Add warning message
    //     printf("Unexisting track!");      
    //     return;
    // }
    // data_storage::queries::table_insert_query query;
    // data_storage::database::get_instance()
    //                         .execute_query(
    //                             query.set_table_name("_rocpd_sample")
    //                                 .set_columns("id", "track_id", "timestamp", "event_id")
    //                                 .set_values(_sample_id++, _track_name_map[track_name], timestamp, event_id)
    //                                 .get_query_string());
}

} // namespace rocprofsys
