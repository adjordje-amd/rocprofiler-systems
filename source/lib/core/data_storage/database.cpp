#include "database.hpp"
#include "debug.hpp"

#include <timemory/environment/types.hpp>
#include <chrono>
#include <fstream>
#include <memory>
#include <functional>
#include <filesystem>
#include <regex>

#define USE_RAM_DB 1

namespace fs = std::filesystem;

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
        
        ROCPROFSYS_VERBOSE(0, "Database: %s\r\n", db_name.c_str());
#ifdef USE_RAM_DB
        validate_sqlite3_result(sqlite3_open(":memory:", &_ram_sqlite_db), "database open failed!");
        validate_sqlite3_result(sqlite3_open(db_name.c_str(), &_sqlite3_db), "database open failed!");
#else
        validate_sqlite3_result(sqlite3_open(db_name.c_str(), &_ram_sqlite_db), "database open failed!");
#endif
    };
    
    database::~database() {   
#ifdef USE_RAM_DB  
        auto backup = sqlite3_backup_init(_sqlite3_db, "main", _ram_sqlite_db, "main");
        if (backup) {
            sqlite3_backup_step(backup, -1);  // Copy all pages
            sqlite3_backup_finish(backup);
        }
        sqlite3_close(_ram_sqlite_db);
        sqlite3_close(_sqlite3_db);
#else
        sqlite3_close(_ram_sqlite_db);
#endif
    }

    void database::initialize_schema() {
        auto get_file_path = [](const char* filename) {
            auto _rocprofsys_root = tim::get_env<std::string>(
                "rocprofiler_systems_ROOT", tim::get_env<std::string>("ROCPROFSYS_ROOT", ""));
            if(!_rocprofsys_root.empty() && fs::exists(std::string(_rocprofsys_root)))
            {
                auto new_file_path = std::string(_rocprofsys_root).append("/share/rocprofiler-systems/").append(filename);
                if(fs::exists(new_file_path)) {
                    return new_file_path;
                }
            }
            return std::string("rocprof-sys-source/source/lib/core/data_storage/schema/").append(filename);
        };

        std::ifstream file(get_file_path("tableSchema.sql"));
        if (!file.is_open()){
            throw std::runtime_error("Failed to open database schema file!");
        }

        std::stringstream ss_query;
        ss_query << file.rdbuf();
        std::regex upid_pattern("\\{\\{upid\\}\\}");  
        std::string query = std::regex_replace(ss_query.str(), upid_pattern, "_R4nd0m1D");
        validate_sqlite3_result(sqlite3_exec(_ram_sqlite_db, query.c_str(), 0, 0, 0), "Invalid database schema file, init database failed!");
        file.close();
        
        file.open(get_file_path("utilitySchema.sql"));
        if (!file.is_open()){
            throw std::runtime_error("Failed to open utility schema file!");
        }
        ss_query.str("");
        ss_query << file.rdbuf();
        std::regex view_upid_pattern("\\{\\{view_upid\\}\\}");  
        query = std::regex_replace(ss_query.str(), view_upid_pattern, "_R4nd0m1D");
        validate_sqlite3_result(sqlite3_exec(_ram_sqlite_db, query.c_str(), 0, 0, 0), "Invalid database utility file, init database failed!");
        file.close();
    }

    void database::execute_query(const std::string& query) {
        validate_sqlite3_result(sqlite3_exec(_ram_sqlite_db, query.c_str(), 0, 0, 0), "Failed to execute query - ", query);
    }
}