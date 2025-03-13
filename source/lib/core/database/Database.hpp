#pragma once
#include "queries/TableInsertQuery.hpp"

#include <memory>
#include <sqlite3.h>


namespace database {

class Database {
public:
    Database();

    Database(Database&) = delete;
    Database& operator=(Database&) = delete;
    
    ~Database();
    
    void initialize_schema();

    void execute_query(const std::string& query);

private:
    sqlite3* _sqlite3_db;
};

}

    