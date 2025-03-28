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
    
struct data_processor {

    using insert_event_stmt = std::function<void(uint32_t, uint32_t, uint32_t, uint32_t, const char*, const char*, const char*, const char*, const char*)>;
    using insert_pmc_event_stms = std::function<void(uint32_t, uint32_t, double, const char*)>;


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

    static data_processor& get_instance();

    size_t insert_string(const char* str);

    void insert_agent(const agent_descriptor& agent);
    
    void insert_track(const char* name, uint32_t node_id, pid_t pid, std::thread::id tid);
    
    void initialize_event_stmt();

    size_t insert_event(const data_processing::event& event);

    void initialize_pmc_event_stmt();

    void insert_pmc_event(const data_processing::pmc_event& event);
        
    void insert_sample(const char* track_name, uint64_t timestamp, size_t event_id);

private:
    data_processor();

    data_processor(data_processor&) = delete;

    data_processor& operator=(const data_processor&) = delete;

private:
    std::unordered_map<std::string, uint32_t> _track_name_map;

    insert_event_stmt _insert_event_statement;
    insert_pmc_event_stms _insert_pmc_event_statement;
};

} // namespace rocprofsys

