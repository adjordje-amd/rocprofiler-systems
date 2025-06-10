#pragma once

#include "common/traits.hpp"

#include <iostream>
#include <sstream>
#include <string_view>
#include <type_traits>

namespace rocprofsys
{
namespace rocpd
{
namespace data_storage
{
namespace queries
{
namespace query_builders
{

struct query_value_builder
{
    query_value_builder(std::stringstream& ss)
    : _ss{ ss }
    {}

    template <typename... Values>
    query_value_builder& set_values(Values&&... values)
    {
        auto i = sizeof...(values);
        _ss << "( ";
        ((_ss << (common::traits::is_string_literal<
                      std::remove_reference_t<decltype(values)>>::value
                      ? "\""
                      : "")
              << values
              << (common::traits::is_string_literal<
                      std::remove_reference_t<decltype(values)>>::value
                      ? "\""
                      : "")
              << (i-- > 1 ? ", " : " ")),
         ...)
            << ")";
        return *this;
    }

    std::string get_query_string() { return _ss.str(); }

private:
    std::stringstream& _ss;
};

struct query_columns_builder
{
    query_columns_builder(std::stringstream& ss)
    : _ss{ ss }
    , _query_value_builder{ _ss }
    {}

    template <typename... Columns,
              typename = std::enable_if_t<
                  (common::traits::is_string_literal<Columns>::value && ...)>>
    query_value_builder& set_columns(Columns&... columns)
    {
        auto i = sizeof...(columns);
        _ss << "( ";
        ((_ss << columns << (i-- > 1 ? ", " : " ")), ...) << ") VALUES ";
        return _query_value_builder;
    }

private:
    std::stringstream&  _ss;
    query_value_builder _query_value_builder;
};

}  // namespace query_builders
}  // namespace queries
}  // namespace data_storage
}  // namespace rocpd
}  // namespace rocprofsys
