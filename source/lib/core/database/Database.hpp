#pragma once
#include "queries/TableInsertQuery.hpp"
#include <sqlite3.h>

namespace database {


class Database {
public:
    static Database& get_instance();

    Database(Database&) = delete;
    Database& operator=(Database&) = delete;
    
    ~Database();

private:
    Database();

public:  
    
    void initialize_schema();

    void execute_query(const std::string& query);

private:
    sqlite3* _sqlite3_db;
};

}

    