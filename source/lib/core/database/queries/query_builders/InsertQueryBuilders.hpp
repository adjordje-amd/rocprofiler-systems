#include <memory>
#include <sstream>
#include <type_traits>

#include <iostream>

namespace {
    template <typename T>  
    struct is_string_literal : std::false_type {};  
      
    template <std::size_t N>  
    struct is_string_literal<char[N]> : std::true_type {};  
      
    template <std::size_t N>  
    struct is_string_literal<const char[N]> : std::true_type {}; 
}

namespace database {
namespace queries {
namespace query_builders {

    struct QueryPropertyValueBuilder {
        QueryPropertyValueBuilder(std::stringstream& ss) : _ss{std::move(ss)}{}

        template<typename ... Values>
        std::string set_values(Values&& ... values) {
            auto i = sizeof ...(values);
            _ss << "VALUES ( ";
            ((_ss << (is_string_literal<std::remove_reference_t<decltype(values)>>::value ? "\"" : "")  
                    << values  
                    << (is_string_literal<std::remove_reference_t<decltype(values)>>::value ? "\"" : "")  
                    << (i-- > 1 ? ", " : " ")), ...) << ")";       
            return _ss.str();
        }

    private:
        std::stringstream _ss;  
    };


    struct QueryPropertyNameBuilder {
        QueryPropertyNameBuilder(std::stringstream& ss) : _ss{std::move(ss)}{}

        template <typename ... Columns, typename = std::enable_if_t<(is_string_literal<Columns>::value && ...)>>
        std::unique_ptr<QueryPropertyValueBuilder> set_columns(Columns& ... columns) {
            auto i = sizeof ...(columns);
            _ss << "( ";
            (( _ss << columns << (i-- > 1 ? ", " : " ")), ...) << ") ";  
            return std::make_unique<QueryPropertyValueBuilder>(_ss);
        }

    private:
        std::stringstream _ss;
    };

} // namespace query_builders
} // namespace queries
} // namespace database


