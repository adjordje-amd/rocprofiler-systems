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

#include "cache_storage_parser.hpp"
#include "debug.hpp"
#include "sample_cache/sample_type.hpp"
#include <cstdio>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>

namespace rocprofsys
{
namespace sample_cache
{

constexpr auto file_descriptor_error_code = -1;

void
storage_parser::register_type_callback(
    const entry_type&                                           type,
    const std::function<void(const storage_parsed_type_base&)>& callback)
{
    m_callbacks[type].push_back(callback);
}

void
storage_parser::consume_storage()
{
    struct stat file_stat;
    int         fd = open(filename.c_str(), O_RDONLY);

    if(fd == file_descriptor_error_code)
    {
        std::stringstream ss;
        ss << "Error opening file descriptor for reading: " << filename << "\n";
        throw std::runtime_error(ss.str());
    }

    auto fstat_error_code = fstat(fd, &file_stat);

    if(fstat_error_code == file_descriptor_error_code)
    {
        std::stringstream ss;
        ss << "Error opening file descriptor for reading: " << filename << "\n";
        throw std::runtime_error(ss.str());
    }

    auto* mmap_buffer =
        (uint8_t*) mmap(nullptr, file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    if(mmap_buffer == MAP_FAILED)
    {
        std::stringstream ss;
        ss << "Error opening file for reading: " << filename << "\n";
        throw std::runtime_error(ss.str());
    }

    close(fd);
    bool _parsing_needed = !m_callbacks.empty();

    struct __attribute__((packed)) sample_header
    {
        entry_type type;
        size_t     sample_size;
    };

    sample_header header;

    decltype(file_stat.st_size) file_offset = 0;

    auto is_eof = [&offset = file_offset, &size = file_stat.st_size]() {
        return offset >= size;
    };

    while(!is_eof() && _parsing_needed)
    {
        header = *reinterpret_cast<sample_header*>(mmap_buffer + file_offset);
        file_offset += sizeof(header);

        if(header.sample_size == 0 || is_eof())
        {
            continue;
        }

        // std::vector<uint8_t> sample;
        // sample.reserve(header.sample_size);
        // std::memcpy(sample.data(), mmap_buffer + file_offset, header.sample_size);
        auto* sample_buffer = mmap_buffer + file_offset;
        file_offset += header.sample_size;

        switch(header.type)
        {
            case entry_type::kernel_dispatch:
            {
                kernel_dispatch_sample _kernel_dispatch_sample;
                parse_data(sample_buffer, _kernel_dispatch_sample.record,
                           _kernel_dispatch_sample.stream_handle);

                invoke_callbacks(header.type, _kernel_dispatch_sample);
                break;
            }
            case entry_type::memory_copy:
            {
                memory_copy_sample _memory_copy_sample;
                parse_data(sample_buffer, _memory_copy_sample.record,
                           _memory_copy_sample.stream_handle);
                invoke_callbacks(header.type, _memory_copy_sample);
                break;
            }
            case entry_type::memory_alloc:
            {
                memory_allocate_sample _memory_allocate_sample;
                parse_data(sample_buffer, _memory_allocate_sample.record,
                           _memory_allocate_sample.stream_handle);

                invoke_callbacks(header.type, _memory_allocate_sample);
                break;
            }
            case entry_type::region:
            {
                region_sample _region_sample;
                parse_data(sample_buffer, _region_sample.record,
                           _region_sample.start_timestamp, _region_sample.end_timestamp,
                           _region_sample.call_stack, _region_sample.args_str,
                           _region_sample.category);

                invoke_callbacks(header.type, _region_sample);
                break;
            }
            case entry_type::in_time_sample:
            {
                in_time_sample _in_time_sample;
                parse_data(sample_buffer, _in_time_sample.track_name,
                           _in_time_sample.timestamp_ns, _in_time_sample.event_metadata,
                           _in_time_sample.stack_id, _in_time_sample.parent_stack_id,
                           _in_time_sample.correlation_id, _in_time_sample.call_stack,
                           _in_time_sample.line_info);
                invoke_callbacks(header.type, _in_time_sample);
                break;
            }
            case entry_type::pmc_event_with_sample:
            {
                pmc_event_with_sample _pmc_event_with_sample;
                parse_data(
                    sample_buffer, _pmc_event_with_sample.track_name,
                    _pmc_event_with_sample.timestamp_ns,
                    _pmc_event_with_sample.event_metadata,
                    _pmc_event_with_sample.stack_id,
                    _pmc_event_with_sample.parent_stack_id,
                    _pmc_event_with_sample.correlation_id,
                    _pmc_event_with_sample.call_stack, _pmc_event_with_sample.line_info,
                    _pmc_event_with_sample.agent_handle,
                    _pmc_event_with_sample.pmc_info_name, _pmc_event_with_sample.value);
                invoke_callbacks(header.type, _pmc_event_with_sample);
                break;
            }
            default: break;
        }
    }

    munmap(mmap_buffer, file_stat.st_size);
    std::remove(filename.c_str());
}

void
storage_parser::invoke_callbacks(entry_type type, const storage_parsed_type_base& parsed)
{
    auto _callback_list = m_callbacks.find(type);
    if(_callback_list == m_callbacks.end())
    {
        ROCPROFSYS_VERBOSE(1, "Callback not found for cache postprocessing");
        return;
    }

    for(auto& cb : _callback_list->second)
    {
        cb(parsed);
    }
}
}  // namespace sample_cache
}  // namespace rocprofsys
