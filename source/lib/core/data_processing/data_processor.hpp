#pragma once 

#include <string>
#include <cstdint>
#include <thread>
#include <unordered_map>
#include <functional>
#include <any>

#include "utils.hpp"
#include "core/data_storage/database.hpp"
#include "core/data_storage/queries/table_insert_query.hpp"


namespace rocprofsys {
namespace {
    using add_event_stmt = std::function<void(int, uint32_t, uint32_t, uint32_t, uint32_t)>;
}

struct data_processor {
    enum class category_id {
        smi_device_busy,
        smi_device_temperature,
        smi_device_power,
        smi_device_memory_usage,
    };

    enum class correlation_id {
        smi_unused
    };

    enum class agent_type {
        cpu,
        gpu,
        unknown
    };

    struct agent_descriptor {
        uint64_t id;
        uint32_t node_id;
        agent_type type;
        uint32_t absolute_index;
        uint32_t logical_index;
        uint32_t type_index;
        uint32_t uuid;
        const char* name;
        const char* model_name;
        const char* vendor_name;
        const char* product_name;
        const char* user_name;
        const char* extdata;
    };

    struct event_descriptor {
       int category_id;
       int correlation_id;
       int stack_id;
       int parent_stack_id;
       const char* args;
       const char* metrics;
       const char* call_stack;
       const char* line_info;
       const char* extdata;
    };

    static data_processor& get_instance();

    int create_string(std::string_view str);

    void create_agent(const agent_descriptor& agent);
    
    void create_track(std::string_view name, uint32_t node_id, pid_t pid, std::thread::id tid);
    
    /**
     * @brief record event into database
     * 
     * This function logs or registers an event with the given category, correlation, stack, and
     * parent stack identifiers. It is typically used in systems where events are categorized
     * and correlated in structured formats for processing, logging, or analysis.
     * 
     * @param category_id A unique identifier representing the category of the event.
     * 
     * @param correlation_id A unique identifier used to correlate this event with others.
     * 
     * @param stack_id A unique identifier representing the stack trace or context from
     * 
     * @param parent_stack_id A unique identifier for the parent stack trace or context.
     */
    template<typename T>
    void add_event(const data_processing::event<T>& event) {
        static auto _add_event_stmt = []{
            data_storage::queries::table_insert_query query_builder;
            auto query = query_builder.set_table_name("rocpd_event")
                                        .set_columns(
                                            "category_id", "correlation_id", "stack_id", "parent_stack_id", 
                                            "args", "metrics", "call_stack", "line_info", "extdata")
                                        .set_values('?', '?', '?', '?', '?', '?', '?', '?', '?')
                                        .get_query_string();
            return data_storage::database::get_instance().create_statment_executor<decltype(event.category_id),
                                                                                    decltype(event.correlation_id),
                                                                                    decltype(event.stack_id),
                                                                                    decltype(event.parent_stack_id),
                                                                                    decltype(event.args),
                                                                                    const char*,
                                                                                    decltype(event.call_stack),
                                                                                    decltype(event.line_info),
                                                                                    decltype(event.extdata)>(query);                                            
        }();

        static std::string metrics;
        metrics = event.metrics.serialize();
        std::cout << "Add event. Metrics " << metrics << std::endl;

        _add_event_stmt(event.category_id, event.correlation_id, event.stack_id, event.parent_stack_id, 
                        event.args, metrics.c_str(), event.call_stack, event.line_info, event.extdata);
    }
        
    void add_sample(std::string_view track_name, uint64_t timestamp);

private:
    data_processor();

    data_processor(data_processor&) = delete;

    data_processor& operator=(const data_processor&) = delete;

private:
    std::unordered_map<std::string_view, uint32_t> _track_name_map;
    std::unordered_map<category_id, int> _category_map;

    uint32_t _track_id{1};
    uint32_t _string_id{1};
    uint32_t _sample_id{1};
    uint32_t _event_id{1};
};

} // namespace rocprofsys

