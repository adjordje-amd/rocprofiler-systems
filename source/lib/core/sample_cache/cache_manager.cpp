// MIT License
//
// Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "cache_manager.hpp"
#include "core/config.hpp"
#include "core/sample_cache/cache_storage_parser.hpp"
#include "debug.hpp"
#include "rocpd/data_storage/database.hpp"
#include "sample_cache/rocpd_post_processing.hpp"
#include <chrono>

namespace rocprofsys
{
namespace sample_cache
{

cache_manager&
cache_manager::get_instance()
{
    static cache_manager instance;
    return instance;
}

cache_manager::cache_manager()
: m_postprocessing{ m_metadata }
{
    m_postprocessing.register_parser_callback(m_parser);
}

void
cache_manager::post_process()
{
    using clock = std::chrono::high_resolution_clock;
    if(m_storage.is_running())
    {
        ROCPROFSYS_WARNING(2, "Postprocessing called without previously shutting down "
                              "cache storage. Calling shutdown explicitly..\n");
        shutdown();
    }

    if(get_use_rocpd())
    {
        ROCPROFSYS_PRINT(
            "Generating rocpd with collected data. This may take a while..\n");
    }

    long metadata_time = 0;
    long cache_time    = 0;

    {
        auto start = clock::now();
        rocpd::data_storage::database::get_instance().execute_query("BEGIN TRANSACTION;");
        post_process_metadata();
        rocpd::data_storage::database::get_instance().execute_query("COMMIT;");
        auto end = clock::now();
        metadata_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    }

    // rocpd::data_storage::database::get_instance().execute_query(
    //     "PRAGMA journal_mode = OFF;");
    // rocpd::data_storage::database::get_instance().execute_query(
    //     "PRAGMA temp_store = MEMORY;");
    // rocpd::data_storage::database::get_instance().execute_query(
    //     "PRAGMA foreign_keys = OFF;");
    // rocpd::data_storage::database::get_instance().execute_query(
    //     "PRAGMA synchronous = OFF;");
    // rocpd::data_storage::database::get_instance().execute_query(
    //     "PRAGMA auto_vacuum = OFF;");

    {
        auto start = clock::now();
        m_parser.consume_storage();
        auto end = clock::now();
        cache_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    }
    m_postprocessing.flush_remaining();

    std::cout << "Metadata time: " << metadata_time
              << " milliseconds Cache time: " << cache_time << " milliseconds"
              << std::endl;
}

void
cache_manager::post_process_metadata()
{
    m_postprocessing.post_process_metadata();
}

void
cache_manager::shutdown()
{
    m_storage.shutdown();
}

}  // namespace sample_cache
}  // namespace rocprofsys
