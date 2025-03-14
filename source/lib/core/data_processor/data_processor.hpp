#pragma once 

#include <string>
#include <cstdint>

namespace rocrpofsys{
    
struct data_processor{

    int create_string(std::string_view str);

    void create_track(std::string_view name, uint32_t node_id, uint32_t pid, uint32_t tid);

    void create_event(uint32_t category_id, uint32_t correlation_id, 
                        uint32_t stack_id, uint32_t parent_stack_id);

    void create_sample(uint32_t track_id, uint64_t timestamp, uint32_t event_id);
private:
    uint32_t _track_id{1};
    uint32_t _string_id{1};
    uint32_t _sample_id{1};
    uint32_t _event_id{1};
};

} // namespace rocrofsys

