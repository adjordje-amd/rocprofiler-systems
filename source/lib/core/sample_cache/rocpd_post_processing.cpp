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

#include "sample_cache/rocpd_post_processing.hpp"
#include "common.hpp"
#include "library/thread_info.hpp"
#include "rocpd/agent_manager.hpp"
#include "rocpd/node_info.hpp"
#include "sample_cache/cache_post_processing.hpp"
#include "sample_cache/cache_storage_parser.hpp"
#include "sample_cache/metadata_storage.hpp"
#include <stdexcept>
#include <timemory/utility/demangle.hpp>

namespace rocprofsys
{
namespace sample_cache
{
namespace
{
rocpd::data_processor&
get_data_processor()
{
    return rocpd::data_processor::get_instance();
}

}  // namespace

postprocessing_callback
rocpd_post_processing::get_kernel_dispatch_callback() const
{
    return [&](const storage_parsed_type_base& parsed) {
        auto _kd = static_cast<const struct kernel_dispatch_sample&>(parsed);

        auto& data_processor = get_data_processor();
        auto& agent_manager  = rocpd::agent_manager::get_instance();
        auto& n_info         = node_info::get_instance();
        auto  process        = m_metadata.get_process_info();
        auto  agent_primary_key =
            agent_manager.get_agent_by_global_id(_kd.agent_id).base_id;

        auto thread_primary_key = m_rocpd_thread_mapping.at(_kd.thread_id);

        auto category_id =
            m_rocpd_string_mapping.at(trait::name<category::rocm_kernel_dispatch>::value);

        auto kernel_symbol = m_metadata.get_kernel_symbol(_kd.kernel_id);

        if(!kernel_symbol.has_value())
        {
            throw std::runtime_error("Kernel symbol is missing for kernel dispatch");
            return;
        }

        auto region_name_primary_key =
            m_rocpd_string_mapping.at(tim::demangle(kernel_symbol->kernel_name));

        auto event_id = data_processor.insert_event(
            category_id, _kd.event_stack_id, _kd.event_parent_stack_id,
            _kd.event_correlation_id, _kd.event_call_stack.c_str());

        data_processor.insert_kernel_dispatch(
            n_info.id, process.pid, thread_primary_key, agent_primary_key, _kd.kernel_id,
            _kd.dispatch_id, _kd.queue_handle, _kd.stream_handle, _kd.start_timestamp,
            _kd.end_timestamp, _kd.private_segment_size, _kd.group_segment_size,
            _kd.workgroup_size_x, _kd.workgroup_size_y, _kd.workgroup_size_z,
            _kd.grid_size_x, _kd.grid_size_y, _kd.grid_size_z, region_name_primary_key,
            event_id);
    };
}

postprocessing_callback
rocpd_post_processing::get_memory_copy_callback() const
{
    return [&](const storage_parsed_type_base& parsed) {
        auto _mc = static_cast<const struct memory_copy_sample&>(parsed);

        auto& data_processor = get_data_processor();
        auto& agent_manager  = rocpd::agent_manager::get_instance();
        auto& n_info         = node_info::get_instance();
        auto  process        = m_metadata.get_process_info();

        auto _name =
            std::string{ m_metadata.get_buffer_name_info().at(_mc.kind, _mc.operation) };
        auto name_primary_key = m_rocpd_string_mapping.at(_name);

        auto category_primary_key =
            m_rocpd_string_mapping.at(trait::name<category::rocm_memory_copy>::value);

        auto thread_primary_key = m_rocpd_thread_mapping.at(_mc.thread_id);

        auto dst_agent_primary_key =
            agent_manager.get_agent_by_global_id(_mc.dst_agent_id).base_id;
        auto src_agent_primary_key =
            agent_manager.get_agent_by_global_id(_mc.src_agent_id).base_id;

        auto event_primary_key = data_processor.insert_event(
            category_primary_key, _mc.stack_id, _mc.parent_stack_id, _mc.correlation_id);

        data_processor.insert_memory_copy(
            n_info.id, process.pid, thread_primary_key, _mc.start_timestamp,
            _mc.end_timestamp, name_primary_key, dst_agent_primary_key, _mc.dst_address,
            src_agent_primary_key, _mc.src_address, _mc.bytes, _mc.queue_handle,
            _mc.stream_handle, name_primary_key, event_primary_key);
    };
}

postprocessing_callback
rocpd_post_processing::get_memory_allocate_callback() const
{
    return [](const storage_parsed_type_base& parsed) {
        auto memory_allocate_sample =
            static_cast<const struct memory_allocate_sample&>(parsed);
        (void) memory_allocate_sample;
    };
}

postprocessing_callback
rocpd_post_processing::get_region_callback() const
{
    return [&](const storage_parsed_type_base& parsed) {
        auto region_sample = static_cast<const struct region_sample&>(parsed);

        (void) region_sample;
    };
}

rocpd_post_processing::rocpd_post_processing(metadata& md)
: m_metadata(md)
{}

void
rocpd_post_processing::register_parser_callback(storage_parser& parser)
{
    parser.register_type_callback(entry_type::region, get_region_callback());
    parser.register_type_callback(entry_type::kernel_dispatch,
                                  get_kernel_dispatch_callback());
    parser.register_type_callback(entry_type::memory_copy, get_memory_copy_callback());
    parser.register_type_callback(entry_type::memory_alloc,
                                  get_memory_allocate_callback());
};

void
rocpd_post_processing::post_process_metadata()
{
    auto& data_processor = get_data_processor();
    auto& agent_mngr     = rocpd::agent_manager::get_instance();
    auto  n_info         = node_info::get_instance();

    data_processor.insert_node_info(n_info.id, n_info.hash, n_info.machine_id.c_str(),
                                    n_info.system_name.c_str(), n_info.node_name.c_str(),
                                    n_info.release.c_str(), n_info.version.c_str(),
                                    n_info.machine.c_str(), n_info.domain_name.c_str());

    auto process_info = m_metadata.get_process_info();
    data_processor.insert_process_info(n_info.id, process_info.ppid, process_info.pid, 0,
                                       0, 0, 0, process_info.command.c_str(), "{}");

    const auto& agents = agent_mngr.get_agents();
    for(const auto& rocpd_agent : agents)
    {
        auto _base_id = rocpd::data_processor::get_instance().insert_agent(
            n_info.id, getpid(),
            ((rocpd_agent->agent->type == ROCPROFILER_AGENT_TYPE_GPU) ? "GPU" : "CPU"),
            rocpd_agent->agent->node_id, rocpd_agent->agent->logical_node_id,
            rocpd_agent->agent->logical_node_type_id, rocpd_agent->agent->device_id,
            rocpd_agent->agent->name, rocpd_agent->agent->model_name,
            rocpd_agent->agent->vendor_name, rocpd_agent->agent->product_name, "");
        rocpd_agent->base_id = _base_id;
    }
    auto _string_list = m_metadata.get_string_list();
    for(auto& _string : _string_list)
    {
        rocpd_insert_string(_string);
    }

    auto _thread_info_list = m_metadata.get_thread_info_list();
    for(auto& t_info : _thread_info_list)
    {
        rocpd_insert_thread_id(t_info, n_info, process_info);
    }

    auto _track_info_list = m_metadata.get_track_info_list();
    for(auto& track : _track_info_list)
    {
        data_processor.insert_track(track.track_name.c_str(), n_info.id, process_info.pid,
                                    m_rocpd_thread_mapping.at(track.thread_id));
    }

    auto _code_object_list = m_metadata.get_code_object_list();
    for(const auto& code_object : _code_object_list)
    {
        auto dev_id = agent_mngr.get_agent_by_handle(code_object.agent_id.handle).base_id;

        const char* strg_type = "UNKNOWN";
        switch(code_object.storage_type)
        {
            case ROCPROFILER_CODE_OBJECT_STORAGE_TYPE_FILE: strg_type = "FILE"; break;
            case ROCPROFILER_CODE_OBJECT_STORAGE_TYPE_MEMORY: strg_type = "MEMORY"; break;
            default: break;
        }
        data_processor.insert_code_object(code_object.code_object_id, n_info.id,
                                          process_info.pid, dev_id, code_object.uri,
                                          code_object.load_base, code_object.load_size,
                                          code_object.load_delta, strg_type);
    }

    auto _kernel_symbols_list = m_metadata.get_kernel_symbol_list();
    for(const auto& kernel_symbol : _kernel_symbols_list)
    {
        auto kernel_name = tim::demangle(kernel_symbol.kernel_name);
        data_processor.insert_kernel_symbol(
            kernel_symbol.kernel_id, n_info.id, process_info.pid,
            kernel_symbol.code_object_id, kernel_symbol.kernel_name, kernel_name.c_str(),
            kernel_symbol.kernel_object, kernel_symbol.kernarg_segment_size,
            kernel_symbol.kernarg_segment_alignment, kernel_symbol.group_segment_size,
            kernel_symbol.private_segment_size, kernel_symbol.sgpr_count,
            kernel_symbol.arch_vgpr_count, kernel_symbol.accum_vgpr_count);

        rocpd_insert_string(kernel_name);
    }

    auto _queue_list = m_metadata.get_queue_list();
    for(const auto& queue_handle : _queue_list)
    {
        std::stringstream ss;
        ss << "Queue " << queue_handle;
        data_processor.insert_queue_info(queue_handle, n_info.id, process_info.pid,
                                         ss.str().c_str());
    }

    auto _stream_list = m_metadata.get_stream_list();
    for(const auto& stream_handle : _stream_list)
    {
        std::stringstream ss;
        ss << "Stream " << stream_handle;
        data_processor.insert_stream_info(stream_handle, n_info.id, process_info.pid,
                                          ss.str().c_str());
    }

    // TODO:
    // auto pmc_info = m_metadata.get_pmc_info_list();
};

void
rocpd_post_processing::rocpd_insert_string(const std::string& str)
{
    auto primary_key = get_data_processor().insert_string(str.c_str());
    m_rocpd_string_mapping.emplace(str, primary_key);
}

void
rocpd_post_processing::rocpd_insert_thread_id(info::thread&        t_info,
                                              const node_info&     n_info,
                                              const info::process& process_info)
{
    const auto& extended_info = thread_info::get(t_info.thread_id, SequentTID);
    if(extended_info.has_value())
    {
        t_info.start = extended_info->get_start();
        t_info.end   = extended_info->get_stop();
    }

    std::stringstream ss;
    ss << "Thread " << t_info.thread_id;
    auto primary_key = get_data_processor().insert_thread_info(
        n_info.id, process_info.ppid, process_info.pid, t_info.thread_id,
        ss.str().c_str(), t_info.start, t_info.end);

    m_rocpd_thread_mapping.emplace(t_info.thread_id, primary_key);
}

void
rocpd_post_processing::post_process_cache()
{}

}  // namespace sample_cache
}  // namespace rocprofsys
