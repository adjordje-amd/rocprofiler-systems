#include "data_processor.hpp"
#include "debug.hpp"

namespace rocprofsys {
namespace data_processing {
namespace {

static inline constexpr const char*
get_agent_type(const types::agent_type& agent_type)
{
    switch(agent_type)
    {
        case types::agent_type::cpu: return "CPU";

        case types::agent_type::gpu: return "GPU";

        default: return "UNKNOWN";
    }
}

}  // namespace

data_processor::data_processor()
{
    data_storage::database::get_instance().initialize_schema();

    // Initialize event statement
    initialize_event_stmt();
    initialize_pmc_event_stmt();
    initialize_sample_stmt();

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
data_processor::insert_agent(size_t node_id, const char* agent_type, size_t absolute_index, size_t logical_index, size_t type_index, uint64_t uuid, 
                            const char* name, const char* model_name, const char* vendor_name, const char* product_name, const char* user_name, const char* extdata) {
    data_storage::queries::table_insert_query query;
    data_storage::database::get_instance().execute_query(
        query.set_table_name("rocpd_agent")
            .set_columns("id", "node_id", "type", "absolute_index", "logical_index",
                         "type_index", "uuid", "name", "model_name", "vendor_name",
                         "product_name", "user_name", "extdata")
            .set_values(_agent_id, node_id, agent_type, absolute_index, logical_index, type_index, 
                        uuid, name, model_name, vendor_name, product_name, user_name, extdata)
            .get_query_string());
    _agent_id_map.emplace(_agent_id, node_id);
    _agent_id++;
}

void
data_processor::insert_track(const char* track_name, size_t node_id, int32_t pid, int64_t tid, const char* extdata)
{
    if (_track_name_map.find(track_name) != _track_name_map.end()) {
        // TODO: Add warning message
        ROCPROFSYS_WARNING(0, "Fail to add track %s, already exist!\n", track_name);
        return;
    }

    auto name_id = insert_string(track_name);
    data_storage::queries::table_insert_query query;
    data_storage::database::get_instance()
                            .execute_query(
                                query.set_table_name("_rocpd_track")
                                    .set_columns("node_id", "pid", "tid", "name_id", "extdata")
                                    .set_values(node_id, pid, tid, name_id, extdata) .get_query_string());
    _track_name_map.emplace(track_name, name_id);
}

void
data_processor::insert_pmc_description(const char* target_arch, size_t agent_id, size_t event_code, size_t instance_id, const char* name, const char* symbol, 
                                        const char* description, const char* long_description, const char* component, const char* units, const char* value_type, 
                                        const char* block, const char* expression, uint32_t is_constant, uint32_t is_derived, const char* extdata)
{
    auto it = _pmc_descriptor_map.find({agent_id, name});
    if (it != _pmc_descriptor_map.end()) {
        ROCPROFSYS_WARNING(0, "Insert PMC event failed! Error: PMC descriptor already exist!\n");  
        return;
    }
    data_storage::queries::table_insert_query query_builder;
    auto query = query_builder.set_table_name("rocpd_pmc")
                                .set_columns("id", "target_arch", "agent_id", "event_code", "instance_id",
                                            "name", "symbol", "description", "long_description",
                                            "component", "units", "value_type", "block",
                                            "expression", "is_constant", "is_derived", "extdata")
                                .set_values(_pmc_id, target_arch, agent_id, event_code, instance_id, name, symbol,
                                            description, long_description, component, units, value_type,
                                            block, expression, is_constant, is_derived, extdata)
                                .get_query_string();
    data_storage::database::get_instance().execute_query(query);

    _pmc_descriptor_map.emplace(std::pair<pmc_identifier, size_t>{{agent_id, name}, _pmc_id});
    _pmc_id++;
}

void
data_processor::insert_pmc_event(size_t event_id, size_t agent_id, const char* pmc_name, double value, const char* extdata)
{
    auto it = _pmc_descriptor_map.find({agent_id, pmc_name});
    if (it == _pmc_descriptor_map.end()) {
        ROCPROFSYS_WARNING(0, "Insert PMC event failed! Error: unexisting PMC description agent id: %ld, pmc name: %s !\n", agent_id, pmc_name);  
        return;
    }

    const auto [_, pmc_description_id] = *it;
    _insert_pmc_event_statement(_pmc_event_id, event_id, pmc_description_id, value, extdata);
    _pmc_event_id++;
}

void
data_processor::insert_sample(const char* track, uint64_t timestamp, size_t event_id, const char* extdata)
{
    auto it = _track_name_map.find(track);
    if ( it == _track_name_map.end()) {
        ROCPROFSYS_WARNING(0, "Insert sample failed! Error: Unexisting track %s!\n", track);
        return;
    }
    auto [_, track_id] = *it;
    _insert_sample_statement(track_id, timestamp, event_id, extdata);
}

size_t
data_processor::insert_event(size_t category_id, size_t correlation_id, size_t stack_id,
                            size_t parent_stack_id, const char* args, const char* metrics,
                            const char* call_stack, const char* line_info, const char* extdata)
{
    _insert_event_statement(_event_id, category_id, correlation_id, stack_id,
                            parent_stack_id, args, metrics, call_stack, line_info, extdata);
    return _event_id++;
}

void
data_processor::initialize_event_stmt()
{
    data_storage::queries::table_insert_query query_builder;
    auto query = query_builder.set_table_name("rocpd_event")
                                .set_columns("id", "category_id", "correlation_id", "stack_id", "parent_stack_id",
                                            "args", "metrics", "call_stack", "line_info", "extdata")
                                .set_values('?', '?', '?', '?', '?', '?', '?', '?', '?', '?')
                                .get_query_string();
    _insert_event_statement = data_storage::database::get_instance()
                                                        .create_statment_executor<size_t, size_t, size_t, size_t, uint32_t, const char*,
                                                                                const char*, const char*, const char*, const char*>(query);
}

void
data_processor::initialize_pmc_event_stmt()
{
    data_storage::queries::table_insert_query query_builder;
    auto query = query_builder.set_table_name("rocpd_pmc_event")
                                .set_columns("id", "event_id", "pmc_id", "value", "extdata")
                                .set_values('?', '?', '?', '?', '?')
                                .get_query_string();
    _insert_pmc_event_statement = data_storage::database::get_instance()
                                                        .create_statment_executor<size_t, size_t, size_t, double, const char*>(query);
}

void
data_processor::initialize_sample_stmt()
{
    data_storage::queries::table_insert_query query_builder;
    auto query = query_builder.set_table_name("_rocpd_sample")
                                .set_columns("track_id", "timestamp", "event_id", "extdata")
                                .set_values('?', '?', '?', '?')
                                .get_query_string();
    _insert_sample_statement = data_storage::database::get_instance().create_statment_executor<size_t, uint64_t, size_t, const char*>(query);
}

} // namespace data_processing
}  // namespace rocprofsys
