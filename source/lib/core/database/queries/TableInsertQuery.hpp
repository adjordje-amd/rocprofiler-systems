#pragma once

#include <memory>

#include "query_builders/InsertQueryBuilders.hpp"

namespace database {
namespace queries {

    struct TableInsertQuery {        
        std::unique_ptr<query_builders::QueryPropertyNameBuilder> set_table_name(const std::string& tableName) {
            _ss.clear();
            _ss << "INSERT INTO " << tableName << " ";
            return std::make_unique<query_builders::QueryPropertyNameBuilder>(_ss);
        }

    private:
        std::stringstream _ss;
    };

} // namepsace queries
} // namespace database