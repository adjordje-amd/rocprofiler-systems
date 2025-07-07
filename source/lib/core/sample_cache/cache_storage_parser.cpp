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
#include <mutex>

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
    std::cout << __LINE__ << "register type" << std::endl;
    m_callbacks.emplace(type, callback);
}

void
storage_parser::load_storage(const std::filesystem::path& path)
{
    std::ifstream ifs(path, std::ios::binary | std::ios::ate);
    if(!ifs)
    {
        std::cerr << "Error opening file for writing: " << path << "\n";
        return;
    }

    sample_type type;
    size_t      sample_size;

    std::cout << __LINE__ << "load storage" << std::endl;

    while(!ifs.eof())
    {
        ifs.read(reinterpret_cast<char*>(&type), sizeof(type));
        ifs.read(reinterpret_cast<char*>(&sample_size), sizeof(sample_size));

        std::cout << __LINE__ << "load storage" << std::endl;
        if(sample_size == 0 || ifs.eof())
        {
            continue;
        }

        std::vector<uint8_t> sample;
        sample.reserve(sample_size);
        ifs.read(reinterpret_cast<char*>(sample.data()), sample_size);
        std::cout << __LINE__ << "load storage" << std::endl;

        switch(type)
        {
            case sample_type::kernel_dispatch:
            {
                std::cout << __LINE__ << "load storage" << std::endl;
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
                           _kernel_dispatch_sample.node_info_id);

                if(m_callbacks.count(type) > 0)
                {
                    m_callbacks.at(type)(_kernel_dispatch_sample);
                    std::cout << __LINE__ << "load storage" << std::endl;
                }
                break;
            }
            default: break;
        }
    }

    ifs.close();
}

}  // namespace cache
}  // namespace rocprofsys
