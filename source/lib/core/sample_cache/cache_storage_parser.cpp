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
#include <cstdio>

namespace rocprofsys
{
namespace cache
{
storage_parser&
storage_parser::get_instance()
{
    static storage_parser instance;
    return instance;
}

void
storage_parser::register_type_callback(
    const sample_type&                                          type,
    const std::function<void(const storage_parsed_type_base&)>& callback)
{
    if(m_callbacks.count(type) > 0)
    {
        return;
    }
    m_callbacks.emplace(type, callback);
}

void
storage_parser::consume_storage()
{
    const std::filesystem::path& path = { filename };
    std::ifstream                ifs(path, std::ios::binary);
    if(!ifs)
    {
        std::cerr << "Error opening file for writing: " << path << "\n";
        return;
    }

    sample_type type;
    size_t      sample_size;

    std::map<sample_type, size_t> m_parsed_count;

    while(!ifs.eof())
    {
        ifs.read(reinterpret_cast<char*>(&type), sizeof(type));
        ifs.read(reinterpret_cast<char*>(&sample_size), sizeof(sample_size));

        if(sample_size == 0 || ifs.eof())
        {
            continue;
        }

        std::vector<uint8_t> sample;
        sample.reserve(sample_size);
        ifs.read(reinterpret_cast<char*>(sample.data()), sample_size);

        switch(type)
        {
            case sample_type::kernel_dispatch:
            {
                kernel_dispatch_sample _kernel_dispatch_sample;
                parse_data(sample.data(), _kernel_dispatch_sample.kernel_id,
                           _kernel_dispatch_sample.dispatch_id,
                           _kernel_dispatch_sample.queue_handle,
                           _kernel_dispatch_sample.stream_handle,
                           _kernel_dispatch_sample.start_timestamp,
                           _kernel_dispatch_sample.end_timestamp,
                           _kernel_dispatch_sample.private_segment_size,
                           _kernel_dispatch_sample.group_segment_size,
                           _kernel_dispatch_sample.workgroup_size_x,
                           _kernel_dispatch_sample.workgroup_size_y,
                           _kernel_dispatch_sample.workgroup_size_z,
                           _kernel_dispatch_sample.grid_size_x,
                           _kernel_dispatch_sample.grid_size_y,
                           _kernel_dispatch_sample.grid_size_z,
                           _kernel_dispatch_sample.thread_id,
                           _kernel_dispatch_sample.category,
                           _kernel_dispatch_sample.event_stack_id,
                           _kernel_dispatch_sample.event_parent_stack_id,
                           _kernel_dispatch_sample.event_correlation_id,
                           _kernel_dispatch_sample.event_call_stack,
                           _kernel_dispatch_sample.node_info_id,
                           _kernel_dispatch_sample.agent_id);

                if(m_callbacks.count(type) > 0)
                {
                    m_callbacks.at(type)(_kernel_dispatch_sample);
                }
                break;
            }
            case sample_type::memory_copy:
            {
                memory_copy_sample _memory_copy_sample;
                parse_data(
                    sample.data(), _memory_copy_sample.node_id,
                    _memory_copy_sample.process_id, _memory_copy_sample.thread_id,
                    _memory_copy_sample.start_timestamp,
                    _memory_copy_sample.end_timestamp, _memory_copy_sample.kind,
                    _memory_copy_sample.operation, _memory_copy_sample.dst_agent_id,
                    _memory_copy_sample.dst_address, _memory_copy_sample.src_agent_id,
                    _memory_copy_sample.src_address, _memory_copy_sample.bytes,
                    _memory_copy_sample.queue_handle, _memory_copy_sample.stream_handle,
                    _memory_copy_sample.stack_id, _memory_copy_sample.parent_stack_id,
                    _memory_copy_sample.correlation_id);
                if(m_callbacks.count(type) > 0)
                {
                    m_callbacks.at(type)(_memory_copy_sample);
                }
                break;
            }
            case sample_type::memory_alloc:
            {
                memory_allocate_sample _memory_allocate_sample;
                parse_data(sample.data(), _memory_allocate_sample.node_id,
                           _memory_allocate_sample.process_id,
                           _memory_allocate_sample.thread_id,
                           _memory_allocate_sample.agent_id, _memory_allocate_sample.kind,
                           _memory_allocate_sample.operation,
                           _memory_allocate_sample.start_timestamp,
                           _memory_allocate_sample.end_timestamp,
                           _memory_allocate_sample.address_value,
                           _memory_allocate_sample.allocation_size,
                           _memory_allocate_sample.queue_handle,
                           _memory_allocate_sample.stream_handle,
                           _memory_allocate_sample.stack_id,
                           _memory_allocate_sample.parent_stack_id,
                           _memory_allocate_sample.correalation_id);

                if(m_callbacks.count(type) > 0)
                {
                    m_callbacks.at(type)(_memory_allocate_sample);
                }
                break;
            }
            case sample_type::region:
            {
                region_sample _region_sample;
                parse_data(sample.data(), _region_sample.thread_id,
                           _region_sample.start_timestamp, _region_sample.end_timestamp,
                           _region_sample.kind, _region_sample.operation,
                           _region_sample.stack_id, _region_sample.parent_stack_id,
                           _region_sample.correalation_id, _region_sample.call_stack,
                           _region_sample.args_str);

                if(m_callbacks.count(type) > 0)
                {
                    m_callbacks.at(type)(_region_sample);
                }
                break;
            }
            default: break;
        }
    }

    ifs.close();
    std::filesystem::remove(path);
}

}  // namespace cache
}  // namespace rocprofsys
