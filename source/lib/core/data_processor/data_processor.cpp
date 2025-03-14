#include "data_processor.hpp"
#include "data_storage/database.hpp"

namespace rocrpofsys{

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

void data_processor::create_track(std::string_view name, uint32_t node_id, uint32_t pid, uint32_t tid){
    auto name_id = create_string(name);
    data_storage::queries::table_insert_query query;
    data_storage::database::get_instance()
                            .execute_query(
                                query.set_table_name("_rocpd_track")
                                    .set_columns("id", "node_id", "pid", "tid", "name_id")
                                    .set_values(_track_id++, node_id, pid, tid, name_id)
                                    .get_query_string());
}

void data_processor::create_event(uint32_t category_id, uint32_t correlation_id, 
                                    uint32_t stack_id, uint32_t parent_stack_id) 
{
    data_storage::queries::table_insert_query query;
    data_storage::database::get_instance()
                            .execute_query(
                                query.set_table_name("_rocpd_sample")
                                    .set_columns("id", "category_id", "correlation_id", "stack_id", "parent_stack_id")
                                    .set_values(_event_id++, category_id, correlation_id, stack_id, parent_stack_id)
                                    .get_query_string());
}

void data_processor::create_sample(uint32_t track_id, uint64_t timestamp, uint32_t event_id) {
    data_storage::queries::table_insert_query query;
    data_storage::database::get_instance()
                            .execute_query(
                                query.set_table_name("_rocpd_sample")
                                    .set_columns("id", "track_id", "timestamp", "event_id")
                                    .set_values(_sample_id++, track_id, timestamp, event_id)
                                    .get_query_string());
}

} // namespace rocrofsys
