#pragma once 

#include <type_traits>

namespace data_processing {

template<typename T>
struct serializable {
    const std::string serialize() const{
        return static_cast<const T*>(this)->serialize_impl();
    }
};

template<typename T, typename Enable = void>
class event;

template<typename T>
struct event<T, std::enable_if_t<std::is_base_of<serializable<T>, T>::value>> {
    event(int category_id, int correlation_id, int stack_id, int parent_stack_id, const char* args, const T& metrics, 
            const char* call_stack, const char* line_info,  const char* extdata) 
            : category_id{category_id}, correlation_id{correlation_id}, stack_id{stack_id}, parent_stack_id{parent_stack_id}, args{args}, 
                metrics{metrics}, call_stack{call_stack}, line_info{line_info}, extdata{extdata} {}
    int category_id;
    int correlation_id;
    int stack_id;
    int parent_stack_id;
    const char* args;
    const T& metrics;
    const char* call_stack;
    const char* line_info;
    const char* extdata;
};

} //namespace data_processing