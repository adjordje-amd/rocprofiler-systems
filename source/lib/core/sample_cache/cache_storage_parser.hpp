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
#include <rocprofiler-systems/categories.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

namespace rocprofsys
{
namespace cache
{
struct storage_parsed_type_base
{};

struct kernel_dispatch_sample : storage_parsed_type_base
{
    uint64_t              kernel_id;
    uint64_t              dispatch_id;
    uint64_t              queue_handle;
    uint64_t              stream_handle;
    uint64_t              start_timestamp;
    uint64_t              end_timestamp;
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
    uint64_t              event_stack_id;
    uint64_t              event_parent_stack_id;
    uint64_t              event_correlation_id;
    std::string           event_call_stack;
    uint64_t              node_info_id;
    size_t                agent_id;
};

struct memory_copy_sample : storage_parsed_type_base
{};

class storage_parser
{
public:
    static storage_parser& get_instance();

    void register_type_callback(
        const sample_type&                                                 type,
        const std::function<void(const cache::storage_parsed_type_base&)>& callback);

    void load_storage(const std::filesystem::path& path = { filename });

private:
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
    std::map<sample_type, std::function<void(const cache::storage_parsed_type_base&)>>
        m_callbacks;
};

}  // namespace cache
}  // namespace rocprofsys
