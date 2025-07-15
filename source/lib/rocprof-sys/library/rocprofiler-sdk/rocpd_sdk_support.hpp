#pragma once

#include "core/rocpd/agent_manager.hpp"
#include "core/rocpd/data_processor.hpp"
#include "core/rocpd/node_info.hpp"
#include "library/rocprofiler-sdk/fwd.hpp"

#include <string>
#include <unistd.h>

namespace rocprofsys
{
namespace rocprofiler_sdk
{
namespace
{

template <typename CorrelationIdType>
uint64_t
get_parent_stack_id_impl(const CorrelationIdType& correlation_id)
{
    if constexpr(std::is_same_v<rocprofiler_correlation_id_t, CorrelationIdType>)
    {
        return correlation_id.ancestor;
    }
    else
    {
        return 0;
    }
}

auto&
get_stream_stack_impl()
{
    static thread_local std::vector<rocprofiler_stream_id_t> _v{ rocprofiler_stream_id_t{
        .handle = 0 } };
    return _v;
}

rocprofiler_stream_id_t
get_stream_id_impl(auto* _record)
{
    auto _stream_id = rocprofiler_stream_id_t{ .handle = 0 };
    if(_record->correlation_id.external.ptr != nullptr)
    {
        // Extract the stream id
        auto* _ecid_data = static_cast<kernel_rename_and_stream_data*>(
            _record->correlation_id.external.ptr);
        _stream_id                             = _ecid_data->stream_id;
        auto _region_id                        = _ecid_data->region_id;
        _record->correlation_id.external.value = _region_id;
        delete _ecid_data;
    }
    return _stream_id;
}

void
rocpd_insert_stream_info_impl(const rocprofiler_stream_id_t& stream_id)
{
    auto& data_processor = rocpd::data_processor::get_instance();
    auto& n_info         = node_info::get_instance();
    auto  stream_name    = stream_id.handle == 0
                               ? "Default Stream"
                               : ("Stream " + std::to_string(stream_id.handle));
    data_processor.insert_stream_info(stream_id.handle, n_info.id, getpid(),
                                      stream_name.c_str(), "{}");
}

void
rocpd_insert_memory_copy_impl(rocprofiler_buffer_tracing_memory_copy_record_t* record,
                              size_t name_id, size_t event_id, size_t region_id,
                              size_t thread_id, const char* extdata = "{}")
{
    auto& data_processor = rocpd::data_processor::get_instance();
    auto& n_info         = node_info::get_instance();
    auto& agent_mngr     = rocpd::agent_manager::get_instance();
    auto  stream_id      = get_stream_id_impl(record);
    auto  dst_agent_id =
        agent_mngr.get_agent_by_handle(record->dst_agent_id.handle).base_id;
    auto src_agent_id =
        agent_mngr.get_agent_by_handle(record->src_agent_id.handle).base_id;

    rocpd_insert_stream_info_impl(stream_id);

    data_processor.insert_memory_copy(
        n_info.id, getpid(), thread_id, record->start_timestamp, record->end_timestamp,
        name_id, dst_agent_id, record->dst_address.handle, src_agent_id,
        record->src_address.handle, record->bytes, 0, stream_id.handle, region_id,
        event_id, extdata);
}

void
hip_stream_callback_impl(rocprofiler_callback_tracing_record_t record,
                         rocprofiler_user_data_t* user_data, void* data)
{
    if(record.kind != ROCPROFILER_CALLBACK_TRACING_HIP_STREAM) return;

    auto* stream_handle_data =
        static_cast<rocprofiler_callback_tracing_hip_stream_data_t*>(record.payload);
    auto stream_id = stream_handle_data->stream_id;

    if(record.operation == ROCPROFILER_HIP_STREAM_CREATE)
    {
        ROCPROFSYS_VERBOSE_F(2, "Entered hip_streams_callback function for "
                                "ROCPROFILER_HIP_STREAM_CREATE");
    }
    else if(record.operation == ROCPROFILER_HIP_STREAM_DESTROY)
    {
        ROCPROFSYS_VERBOSE_F(2, "Entered hip_streams_callback function for "
                                "ROCPROFILER_HIP_STREAM_DESTROY");
    }
    else if(record.operation == ROCPROFILER_HIP_STREAM_SET)
    {
        if(record.phase == ROCPROFILER_CALLBACK_PHASE_ENTER)
        {
            ROCPROFSYS_VERBOSE_F(2, "Entered hip_streams_callback function for "
                                    "ROCPROFILER_HIP_STREAM_SET "
                                    "with ROCPROFILER_CALLBACK_PHASE_ENTER");
            get_stream_stack_impl().emplace_back(stream_id);
        }
        else if(record.phase == ROCPROFILER_CALLBACK_PHASE_EXIT)
        {
            ROCPROFSYS_VERBOSE_F(
                2, "Entered hip_stream_callback function for ROCPROFILER_HIP_STREAM_SET "
                   "with ROCPROFILER_CALLBACK_PHASE_EXIT");
            get_stream_stack_impl().pop_back();
        }
    }
    else
    {
        ROCPROFSYS_FAIL_F("Unknown operation for hip_stream_callback!");
    }
}

}  // namespace
}  // namespace rocprofiler_sdk
}  // namespace rocprofsys