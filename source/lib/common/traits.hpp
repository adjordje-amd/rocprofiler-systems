#pragma once
#include <string>
#include <type_traits>

namespace rocprofsys {
inline namespace common {
namespace traits {
    
    template <typename T>  
    struct is_string_literal : std::false_type {};  
    
    template <std::size_t N>  
    struct is_string_literal<char[N]> : std::true_type {};  
    
    template <std::size_t N>  
    struct is_string_literal<const char[N]> : std::true_type {}; 
    
    template <>  
    struct is_string_literal<std::string_view> : std::true_type {};
    
    template <>  
    struct is_string_literal<const char*> : std::true_type {};

    template <>  
    struct is_string_literal<char*> : std::true_type {};

    template <>  
    struct is_string_literal<std::string> : std::true_type {};

    template <> 
    struct is_string_literal<const std::string> : std::true_type {};

    template <>  
    struct is_string_literal<const char* const> : std::true_type {};

    template <>  
    struct is_string_literal<char* const> : std::true_type {};

    template<typename T>
    inline constexpr bool is_string_literal_v = is_string_literal<T>::value;

} // namespace traits
} // namespace common
} // namespace rocprofsys
