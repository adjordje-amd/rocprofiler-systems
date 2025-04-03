#pragma once 

#include <cstdint>
#include <cstddef>
#include <string>

namespace rocprofsys {
namespace data_processing {
namespace types {

enum class agent_type {
    cpu,
    gpu,
    unknown
};

struct agent {
    size_t id;
    size_t node_id;
    agent_type type;
    size_t absolute_index;
    size_t logical_index;
    size_t type_index;
    uint64_t uuid;
    const char* name;
    const char* model_name;
    const char* vendor_name;
    const char* product_name;
    const char* user_name;
    const char* extdata;
};

struct event {
    size_t category_id;
    size_t correlation_id;
    size_t stack_id;
    size_t parent_stack_id;
    const char* args;
    const char* metrics;
    const char* call_stack;
    const char* line_info;
    const char* extdata;
};


struct sample {
    const char* track_name;     
    uint64_t timestamp;     
    size_t event_id;     
    const char* extdata;    
};


} //namespace types
} //namespace data_processing
} //namespace rocprofsys