#pragma once 

#include <type_traits>

namespace data_processing {

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

struct pmc_event {
    size_t event_id;
    size_t pmc_id;
    double value;
    const char* extdata;
};

} //namespace data_processing