#pragma once

#include "core/rocpd/agent_manager.hpp"
#include "core/rocpd/data_processor.hpp"
#include "core/rocpd/node_info.hpp"
#include "library/rocprofiler-sdk/fwd.hpp"

#include <unistd.h>

namespace rocprofsys
{
namespace rocprofiler_sdk
{
namespace
{

template <typename CorrelationIdType>
uint64_t
get_parent_stack_id_impl([[maybe_unused]] const CorrelationIdType& correlation_id)
{
    return 0;
}

auto&
get_stream_stack_impl()
{
    static thread_local std::vector<rocprofiler_stream_id_t> _v{ rocprofiler_stream_id_t{
        .handle = 0 } };
    return _v;
}

template <typename Tp>
rocprofiler_stream_id_t
get_stream_id_impl([[maybe_unused]] Tp* _record)
{
    // Return just default stream
    return rocprofiler_stream_id_t{ .handle = 0 };
}

void
rocpd_insert_stream_info_impl([[maybe_unused]] const rocprofiler_stream_id_t& stream_id)
{
    auto& data_processor = rocpd::data_processor::get_instance();
    auto& n_info         = node_info::get_instance();
    auto  stream_name    = "Default Stream";
    data_processor.insert_stream_info(0, n_info.id, getpid(), stream_name, "{}");
}

void
rocpd_insert_memory_copy_impl(
    [[maybe_unused]] rocprofiler_buffer_tracing_memory_copy_record_t* record,
    [[maybe_unused]] size_t name_id, [[maybe_unused]] size_t event_id,
    [[maybe_unused]] size_t region_id, [[maybe_unused]] size_t thread_id,
    [[maybe_unused]] const char* extdata = "{}")
{
    // Empty implementation for legacy SDK
}

void
hip_stream_callback_impl([[maybe_unused]] rocprofiler_callback_tracing_record_t record,
                         [[maybe_unused]] rocprofiler_user_data_t*              user_data,
                         [[maybe_unused]] void*                                 data)
{
    // Empty implementation for legacy SDK
}

}  // namespace
}  // namespace rocprofiler_sdk
}  // namespace rocprofsys