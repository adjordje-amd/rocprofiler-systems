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

#pragma once

#include "cache_storage.hpp"
#include "sample_type.hpp"
#include <array>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <rocprofiler-sdk/fwd.h>
#include <rocprofiler-systems/categories.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

namespace rocprofsys
{
namespace sample_cache
{
struct storage_parsed_type_base
{};

struct kernel_dispatch_sample : storage_parsed_type_base
{
    size_t                kernel_id;
    size_t                dispatch_id;
    size_t                queue_handle;
    size_t                stream_handle;
    size_t                start_timestamp;
    size_t                end_timestamp;
    int64_t               private_segment_size;
    int64_t               group_segment_size;
    int64_t               workgroup_size_x;
    int64_t               workgroup_size_y;
    int64_t               workgroup_size_z;
    int64_t               grid_size_x;
    int64_t               grid_size_y;
    int64_t               grid_size_z;
    int64_t               thread_id;
    ROCPROFSYS_CATEGORIES category;
    size_t                event_stack_id;
    size_t                event_parent_stack_id;
    size_t                event_correlation_id;
    std::string           event_call_stack;
    size_t                node_info_id;
    size_t                agent_id;
};

struct memory_copy_sample : storage_parsed_type_base
{
    size_t                              node_id;
    size_t                              process_id;
    rocprofiler_thread_id_t             thread_id;
    rocprofiler_timestamp_t             start_timestamp;
    rocprofiler_timestamp_t             end_timestamp;
    rocprofiler_buffer_tracing_kind_t   kind;
    rocprofiler_memory_copy_operation_t operation;
    size_t                              dst_agent_id;
    size_t                              dst_address;
    size_t                              src_agent_id;
    size_t                              src_address;
    size_t                              bytes;
    size_t                              queue_handle;
    size_t                              stream_handle;
    size_t                              stack_id;
    size_t                              parent_stack_id;
    size_t                              correlation_id;
};

struct memory_allocate_sample : storage_parsed_type_base
{
    size_t                                    node_id;
    size_t                                    process_id;
    rocprofiler_thread_id_t                   thread_id;
    size_t                                    agent_id;
    rocprofiler_buffer_tracing_kind_t         kind;
    rocprofiler_memory_allocation_operation_t operation;
    rocprofiler_timestamp_t                   start_timestamp;
    rocprofiler_timestamp_t                   end_timestamp;
    size_t                                    address_value;
    size_t                                    allocation_size;
    size_t                                    queue_handle;
    size_t                                    stream_handle;
    size_t                                    stack_id;
    size_t                                    parent_stack_id;
    size_t                                    correalation_id;
};

struct region_sample : storage_parsed_type_base
{
    size_t                              thread_id;
    rocprofiler_timestamp_t             start_timestamp;
    rocprofiler_timestamp_t             end_timestamp;
    rocprofiler_callback_tracing_kind_t kind;
    rocprofiler_tracing_operation_t     operation;
    size_t                              stack_id;
    size_t                              parent_stack_id;
    size_t                              correalation_id;
    std::string                         call_stack;
    std::string                         args_str;
};
using postprocessing_callback = std::function<void(const storage_parsed_type_base&)>;
class cache_manager;
class storage_parser
{
public:
    void register_type_callback(const entry_type&              type,
                                const postprocessing_callback& callback);

    void consume_storage();

private:
    friend class cache_manager;
    storage_parser() = default;
    template <typename T>
    static void process_arg(const uint8_t*& data_pos, T& arg)
    {
        if constexpr(std::is_same_v<T, std::string>)
        {
            arg = std::string((const char*) data_pos);
            data_pos += arg.size() + 1;
        }
        else
        {
            arg = *reinterpret_cast<const T*>(data_pos);
            data_pos += sizeof(T);
        }
    }

    template <typename... Args>
    static void parse_data(const uint8_t* data_pos, Args&... args)
    {
        (process_arg(data_pos, args), ...);
    }

private:
    std::map<entry_type, postprocessing_callback> m_callbacks;
};

}  // namespace sample_cache
}  // namespace rocprofsys
