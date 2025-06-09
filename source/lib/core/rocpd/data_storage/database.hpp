#pragma once
#include "queries/table_insert_query.hpp"
#include "common/traits.hpp"
#include <sqlite3.h>
#include <optional>
#include <memory>
#include <functional>
#include <mutex>

namespace rocprofsys {
namespace rocpd {
namespace data_storage {

static std::mutex _mutex;

class database {
public:
    static database& get_instance();

    database(database&) = delete;
    database& operator=(database&) = delete;

    ~database();

private:
    database();

    template<typename ... Args>
    static inline void validate_sqlite3_result(int sqlite3_error_code, Args&& ... args) {
        if (SQLITE_OK != sqlite3_error_code && SQLITE_DONE != sqlite3_error_code) {
            std::stringstream ss;
            auto stream_arg = [&ss](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::optional<size_t>>) {
                    if (arg.has_value()) {
                        ss << arg.value();
                    } else {
                        ss << "NULL";
                    }
                } else {
                    ss << arg;
                }
                ss << " ";
            };
            (stream_arg(args), ...);
            ss << " [Sqlite3 error: " << sqlite3_errstr(sqlite3_error_code) << "]";
            throw std::runtime_error(ss.str());
        }
    }

    template<typename ... Args>
    static inline void validate_sqlite3_result(int sqlite3_error_code, sqlite3* db, Args&& ... args) {
        if (SQLITE_OK != sqlite3_error_code && SQLITE_DONE != sqlite3_error_code) {
            std::stringstream ss;
            auto stream_arg = [&ss](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::optional<size_t>>) {
                    if (arg.has_value()) {
                        ss << arg.value();
                    } else {
                        ss << "NULL";
                    }
                } else {
                    ss << arg;
                }
                ss << " ";
            };
            (stream_arg(args), ...);
            ss << " [Sqlite3 error: " << sqlite3_errstr(sqlite3_error_code);
            ss << " (Extended error message: " << sqlite3_errmsg(db) << ")]";
            throw std::runtime_error(ss.str());
        }
    }

public:

    void initialize_schema();

    void execute_query(const std::string& query);

    size_t get_last_insert_id();

    /**
     * This function prepares an SQLite statement based on the provided SQL query and returns a lambda
     * that can execute the prepared statement, binding the provided values to the respective placeholders
     * in the query.
    */
    template<typename ... Values>
    auto create_statment_executor(const std::string& query) {
        sqlite3_stmt* p_stmt;
        sqlite3* db = _ram_sqlite_db ? _ram_sqlite_db : _sqlite3_db;
        validate_sqlite3_result(sqlite3_prepare_v2(_ram_sqlite_db, query.c_str(), -1, &p_stmt, nullptr), "Failed to create statement!", query);
        std::shared_ptr<sqlite3_stmt> stmt{p_stmt, sqlite3_finalize};

        return [stmt, query, db](Values ... value) {
            std::lock_guard lock { _mutex };
            int position = 1;
            auto bind_value = [&](auto value) {
                using T = decltype(value);
                if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t>) {
                    database::validate_sqlite3_result(sqlite3_bind_int(stmt.get(), position, value), db, "Failed to bind int32_t/uint32_t! Position: ", position, ", Values: ", value);
                }else if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t>) {
                    database::validate_sqlite3_result(sqlite3_bind_int64(stmt.get(), position, value), db, "Failed to bind int64_t/uint64_t! Position: ", position, ", Values: ", value);
                } else if constexpr (std::is_floating_point_v<T>) {
                    database::validate_sqlite3_result(sqlite3_bind_double(stmt.get(), position, value), db, "Failed to bind double! Position: ", position, ", Values: ", value);
                } else if constexpr (common::traits::is_string_literal_v<std::decay_t<T>>) {
                    database::validate_sqlite3_result(sqlite3_bind_text(stmt.get(), position, value, -1, SQLITE_STATIC), db, "Failed to bind text! Position: ", position, ", Values: ", value);
                } else if constexpr (std::is_same_v<std::decay_t<T>, std::optional<size_t>>) {
                    if (value.has_value()) {
                        database::validate_sqlite3_result(sqlite3_bind_int64(stmt.get(), position, value.value()), db, "Failed to bind optional<size_t>! Position: ", position, ", Values: ", value.value());
                    } else {
                        database::validate_sqlite3_result(sqlite3_bind_null(stmt.get(), position), db, "Failed to bind NULL for optional<size_t>! Position: ", position);
                }
                } else {
                    throw std::runtime_error("Unsupported type for binding!");
                }
                position++;
            };

            (bind_value(value),...);

            validate_sqlite3_result(sqlite3_step(stmt.get()), db, "Failed to execute step! Query: ", query, ", Values: ", value...);
            sqlite3_reset(stmt.get());
        };
    }

    static std::string get_upid();

private:
    sqlite3* _sqlite3_db{nullptr};
    sqlite3* _ram_sqlite_db{nullptr};
};

} // namespace data_storage
} // namespace rocpd
} // namespace rocprofsys
