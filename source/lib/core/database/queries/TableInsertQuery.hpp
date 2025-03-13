#pragma once

#include "query_builders/InsertQueryBuilders.hpp"

namespace database {
namespace queries {

    struct TableInsertQuery {     
        TableInsertQuery() : _query_columns_builder{_ss}{}  

        query_builders::QueryColumnsBuilder& set_table_name(const std::string& tableName) {
            _ss.clear();
            _ss << "INSERT INTO " << tableName << " ";
            return _query_columns_builder;
        }

    private:
        std::stringstream _ss;
        query_builders::QueryColumnsBuilder _query_columns_builder;
    };

} // namepsace queries
} // namespace database