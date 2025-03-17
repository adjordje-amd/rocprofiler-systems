#pragma once 

#include <string>
#include <cstdint>
#include <thread>
#include <unordered_map>
#include <functional>
#include <any>

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

    static data_processor& get_instance();

    int create_string(std::string_view str);
    
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
    void add_event(category_id _category_id, correlation_id _correlation_id, 
        uint32_t _stack_id, uint32_t _parent_stack_id);
        
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

