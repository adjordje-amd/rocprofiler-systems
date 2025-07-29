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
#include "core/node_info.hpp"
#include "core/rocpd/data_storage/database.hpp"
#include "core/sample_cache/cache_storage_parser.hpp"
#include "core/sample_cache/metadata_storage.hpp"

namespace rocprofsys
{
namespace sample_cache
{

class rocpd_post_processing
{
public:
    rocpd_post_processing(metadata& metadata);

    void register_parser_callback(storage_parser& parser);
    void post_process_metadata();

    void flush_remaining();

private:
    using primary_key = size_t;

    inline void rocpd_insert_thread_id(info::thread& t_info, const node_info& n_info,
                                       const info::process& process_info) const;

    postprocessing_callback get_kernel_dispatch_callback();
    postprocessing_callback get_memory_copy_callback() const;
    postprocessing_callback get_memory_allocate_callback() const;
    postprocessing_callback get_region_callback() const;
    postprocessing_callback get_in_time_sample_callback() const;
    postprocessing_callback get_pmc_event_with_sample_callback() const;

    metadata& m_metadata;

    // -------- Bulk insert --------

    std::optional<rocpd::data_storage::database::bulk_executor<
        const char*, size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
        uint64_t, uint64_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
        size_t, size_t, size_t, const char*>>
        m_kernel_dispatch_bulk_insert_executor;

    struct kernel_dispatch_insert
    {
        kernel_dispatch_insert(size_t nid_, size_t pid_, size_t tid_, size_t agent_id_,
                               size_t kernel_id_, size_t dispatch_id_, size_t queue_id_,
                               size_t stream_id_, uint64_t start_, uint64_t end_,
                               size_t private_segment_size_, size_t group_segment_size_,
                               size_t workgroup_size_x_, size_t workgroup_size_y_,
                               size_t workgroup_size_z_, size_t grid_size_x_,
                               size_t grid_size_y_, size_t grid_size_z_,
                               size_t region_name_id_, size_t event_id_)
        : nid(nid_)
        , pid(pid_)
        , tid(tid_)
        , agent_id(agent_id_)
        , kernel_id(kernel_id_)
        , dispatch_id(dispatch_id_)
        , queue_id(queue_id_)
        , stream_id(stream_id_)
        , start(start_)
        , end(end_)
        , private_segment_size(private_segment_size_)
        , group_segment_size(group_segment_size_)
        , workgroup_size_x(workgroup_size_x_)
        , workgroup_size_y(workgroup_size_y_)
        , workgroup_size_z(workgroup_size_z_)
        , grid_size_x(grid_size_x_)
        , grid_size_y(grid_size_y_)
        , grid_size_z(grid_size_z_)
        , region_name_id(region_name_id_)
        , event_id(event_id_)
        {}

        size_t   nid;
        size_t   pid;
        size_t   tid;
        size_t   agent_id;
        size_t   kernel_id;
        size_t   dispatch_id;
        size_t   queue_id;
        size_t   stream_id;
        uint64_t start;
        uint64_t end;
        size_t   private_segment_size;
        size_t   group_segment_size;
        size_t   workgroup_size_x;
        size_t   workgroup_size_y;
        size_t   workgroup_size_z;
        size_t   grid_size_x;
        size_t   grid_size_y;
        size_t   grid_size_z;
        size_t   region_name_id;
        size_t   event_id;
    };

    std::vector<kernel_dispatch_insert> m_kds_list = {};
};

}  // namespace sample_cache
}  // namespace rocprofsys
