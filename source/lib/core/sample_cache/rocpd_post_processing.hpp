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
#include "rocpd/node_info.hpp"
#include "sample_cache/cache_post_processing.hpp"
#include "sample_cache/cache_storage_parser.hpp"
#include "sample_cache/metadata_storage.hpp"
#include <unordered_map>

namespace rocprofsys
{
namespace sample_cache
{

class rocpd_post_processing : public post_processing
{
public:
    rocpd_post_processing(metadata& metadata);

    void register_parser_callback(storage_parser& parser) override;
    void post_process_cache() override;
    void post_process_metadata() override;

private:
    using primary_key = size_t;

    void rocpd_insert_string(const std::string& str);
    void rocpd_insert_thread_id(info::thread& t_info, const node_info& n_info,
                                const info::process& process_info);

    postprocessing_callback get_kernel_dispatch_callback() const;
    postprocessing_callback get_memory_copy_callback() const;
    postprocessing_callback get_memory_allocate_callback() const;
    postprocessing_callback get_region_callback() const;

    metadata&                                    m_metadata;
    std::map<size_t, primary_key>                m_rocpd_thread_mapping;
    std::unordered_map<std::string, primary_key> m_rocpd_string_mapping;
};

}  // namespace sample_cache
}  // namespace rocprofsys
