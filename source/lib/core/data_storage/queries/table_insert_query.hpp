#pragma once

#include "query_builders/insert_query_builders.hpp"

namespace data_storage {
namespace queries {

    struct table_insert_query {     
        table_insert_query() : _query_columns_builder{_ss}{}  

        query_builders::query_columns_builder& set_table_name(const std::string& tableName) {
            _ss.str("");
            _ss << "INSERT INTO " << tableName << " ";
            return _query_columns_builder;
        }

    private:
        std::stringstream _ss;
        query_builders::query_columns_builder _query_columns_builder;
    };

} // namepsace queries
} // namespace database