#pragma once
#include "queries/table_insert_query.hpp"
#include <sqlite3.h>

namespace data_storage {

class database {
public:
    static database& get_instance();

    database(database&) = delete;
    database& operator=(database&) = delete;
    
    ~database();

private:
    database();

public:  
    
    void initialize_schema();

    void execute_query(const std::string& query);

private:
    sqlite3* _sqlite3_db;
};

} // namespace data_storage

    