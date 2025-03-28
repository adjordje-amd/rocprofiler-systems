#include "data_processor.hpp"

namespace rocprofsys
{
namespace
{

static inline constexpr const char*
get_agent_type(data_processor::agent_type agent_type)
{
    switch(agent_type)
    {
        case data_processor::agent_type::cpu: return "CPU";

        case data_processor::agent_type::gpu: return "GPU";

        default: return "UNKNOWN";
    }
}

}  // namespace

constexpr const char* CATEHORY_NAME_SMI_DEVICE_BUSY         = "Device Busy";
constexpr const char* CATEHORY_NAME_SMI_DEVICE_MEMORY_USAGE = "Device Memory Usage";
constexpr const char* CATEHORY_NAME_SMI_DEVICE_POWER        = "Device Power";
constexpr const char* CATEHORY_NAME_SMI_DEVICE_TEMPERATURE  = "Device Temperature";

data_processor::data_processor()
{
    data_storage::database::get_instance().initialize_schema();

    // Initialize event statement
    initialize_event_stmt();
    initialize_pmc_event_stmt();

}

data_processor&
data_processor::get_instance()
{
    static data_processor _instance;
    return _instance;
}

size_t
data_processor::insert_string(const char* str)
{
    data_storage::queries::table_insert_query query;
    data_storage::database::get_instance()
                            .execute_query(
                                query.set_table_name("rocpd_string")
                                    .set_columns("string")
                                    .set_values(str)
                                    .get_query_string());

    return data_storage::database::get_instance().get_last_insert_id();
}

void
data_processor::insert_agent(const data_processor::agent_descriptor& agent)
{
    data_storage::queries::table_insert_query query;
    data_storage::database::get_instance().execute_query(
        query.set_table_name("rocpd_agent")
            .set_columns("id", "node_id", "type", "absolute_index", "logical_index",
                         "type_index", "uuid", "name", "model_name", "vendor_name",
                         "product_name", "user_name", "extdata")
            .set_values(agent.id, agent.node_id, get_agent_type(agent.type),
                        agent.absolute_index, agent.logical_index, agent.type_index,
                        agent.uuid, agent.name, agent.model_name, agent.vendor_name,
                        agent.product_name, agent.user_name, agent.extdata)
            .get_query_string());
}

void
data_processor::insert_track(const char* track_name, uint32_t node_id, pid_t pid,
                             std::thread::id tid)
{
    if (_track_name_map.find(track_name) != _track_name_map.end()) {
        // TODO: Add warning message
        printf("Track already exists!");
        return;
    }

    auto name_id = insert_string(track_name);
    data_storage::queries::table_insert_query query;
    data_storage::database::get_instance()
                            .execute_query(
                                query.set_table_name("_rocpd_track")
                                    .set_columns("node_id", "pid", "tid", "name_id")
                                    .set_values(node_id, pid, tid, name_id) .get_query_string());
}

void
data_processor::insert_sample(const char* track_name, uint64_t timestamp, size_t event_id)
{
    if (_track_name_map.find(track_name) == _track_name_map.end()) {
        // TODO: Add warning message
        printf("Unexisting track!");
        return;
    }
    data_storage::queries::table_insert_query query;
    data_storage::database::get_instance()
                            .execute_query(
                                query.set_table_name("_rocpd_sample")
                                    .set_columns("track_id", "timestamp", "event_id")
                                    .set_values(_track_name_map[track_name], timestamp, event_id)
                                    .get_query_string());
}

void
data_processor::initialize_event_stmt()
{
    data_storage::queries::table_insert_query query_builder;
    auto query = query_builder.set_table_name("rocpd_event")
                                .set_columns("category_id", "correlation_id", "stack_id", "parent_stack_id",
                                            "args", "metrics", "call_stack", "line_info", "extdata")
                                .set_values('?', '?', '?', '?', '?', '?', '?', '?', '?')
                                .get_query_string();
    _insert_event_statement = data_storage::database::get_instance()
                                                        .create_statment_executor<uint32_t, uint32_t, uint32_t, uint32_t, const char*,
                                                                                const char*, const char*, const char*, const char*>(query);
}

size_t
data_processor::insert_event(const data_processing::event& event)
{
    _insert_event_statement(event.category_id, event.correlation_id, event.stack_id,
                            event.parent_stack_id, event.args, event.metrics,
                            event.call_stack, event.line_info, event.extdata);
    return data_storage::database::get_instance().get_last_insert_id();
}

void
data_processor::initialize_pmc_event_stmt()
{
    data_storage::queries::table_insert_query query_builder;
    auto query = query_builder.set_table_name("rocpd_event")
                                .set_columns("event_id", "pmc_id", "value", "extdata")
                                .set_values('?', '?', '?', '?')
                                .get_query_string();
    _insert_pmc_event_statement = data_storage::database::get_instance()
                                                        .create_statment_executor<uint32_t, uint32_t, double, const char*>(query);
}

void
data_processor::insert_pmc_event(const data_processing::pmc_event& event)
{
    _insert_pmc_event_statement(event.event_id, event.pmc_id, event.value, event.extdata);
}

}  // namespace rocprofsys
