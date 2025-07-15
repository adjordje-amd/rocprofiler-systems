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
#include "sample_cache/sample_type.hpp"
#include <cstdio>
#include <utility>

namespace rocprofsys
{
namespace sample_cache
{

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
    const std::filesystem::path& path = { filename };
    std::ifstream                ifs(path, std::ios::binary);
    if(!ifs)
    {
        std::cerr << "Error opening file for writing: " << path << "\n";
        return;
    }

    entry_type type;
    size_t     sample_size;

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
            case entry_type::kernel_dispatch:
            {
                kernel_dispatch_sample _kernel_dispatch_sample;
                parse_data(sample.data(), _kernel_dispatch_sample.record,
                           _kernel_dispatch_sample.stream_handle);

                invoke_callbacks(type, _kernel_dispatch_sample);
                break;
            }
            case entry_type::memory_copy:
            {
                memory_copy_sample _memory_copy_sample;
                parse_data(sample.data(), _memory_copy_sample.record,
                           _memory_copy_sample.stream_handle);
                invoke_callbacks(type, _memory_copy_sample);
                break;
            }
            case entry_type::memory_alloc:
            {
                memory_allocate_sample _memory_allocate_sample;
                parse_data(sample.data(), _memory_allocate_sample.record,
                           _memory_allocate_sample.stream_handle);

                invoke_callbacks(type, _memory_allocate_sample);
                break;
            }
            case entry_type::region:
            {
                region_sample _region_sample;
                parse_data(sample.data(), _region_sample.record,
                           _region_sample.start_timestamp, _region_sample.end_timestamp,
                           _region_sample.call_stack, _region_sample.args_str,
                           _region_sample.category);

                invoke_callbacks(type, _region_sample);
                break;
            }
            default: break;
        }
    }

    ifs.close();
    std::filesystem::remove(path);
}

void
storage_parser::invoke_callbacks(entry_type type, const storage_parsed_type_base& parsed)
{
    if(m_callbacks.count(type) > 0)
    {
        auto _callbacks = m_callbacks.at(type);
        for(auto& cb : _callbacks)
        {
            cb(parsed);
        }
    }
}
}  // namespace sample_cache
}  // namespace rocprofsys
