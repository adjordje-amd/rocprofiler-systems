#include "database.hpp"

#include <chrono>
#include <fstream>
#include <memory>
#include <functional>

namespace data_storage {

    database& database::get_instance() {
        static database _instance;
        return _instance;
    }

    database::database() {
        auto db_name = [] {
            auto now = std::chrono::system_clock::now();
            // Convert the time point to a duration since the epoch
            auto time_since_epoch = now.time_since_epoch();
            // Convert the duration to seconds
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(time_since_epoch).count();
            std::stringstream ss;
            ss << "rocprof-" << seconds << ".db";
            return ss.str();
        }();

        printf("DATABASE NAME: %s\r\n", db_name.c_str());
        validate_sqlite3_result(sqlite3_open(db_name.c_str(), &_sqlite3_db), "database open failed!");
    };

    void database::initialize_schema() {
        auto schemaFile = "tableSchema.sql";
        std::ifstream file(schemaFile);
        if (!file.is_open()){
            throw std::runtime_error("Failed to open database schema file!");
        }

        std::stringstream query;
        query << file.rdbuf();
        validate_sqlite3_result(sqlite3_exec(_sqlite3_db, query.str().c_str(), 0, 0, 0), "Invalid database schema file, init database failed!");
        file.close();
        
        file.open("utilitySchema.sql");
        if (!file.is_open()){
            throw std::runtime_error("Failed to open utility schema file!");
        }
        query.str("");
        query << file.rdbuf();
        validate_sqlite3_result(sqlite3_exec(_sqlite3_db, query.str().c_str(), 0, 0, 0), "Invalid database schema file, init database failed!");
        file.close();
    }

    database::~database() {
        sqlite3_close(_sqlite3_db);
    }

    void database::execute_query(const std::string& query) {
        validate_sqlite3_result(sqlite3_exec(_sqlite3_db, query.c_str(), 0, 0, 0), "Failed to execute query - ", query);
    }
}