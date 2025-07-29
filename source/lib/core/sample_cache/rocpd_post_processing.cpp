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
#include "agent_manager.hpp"
#include "common.hpp"
#include "config.hpp"
#include "core/rocpd/data_storage/database.hpp"
#include "core/rocpd/data_storage/queries/query_builders/insert_query_builders.hpp"
#include "core/rocpd/data_storage/queries/table_insert_query.hpp"
#include "library/rocprofiler-sdk/fwd.hpp"
#include "library/thread_info.hpp"
#include "node_info.hpp"
#include "rocpd/data_processor.hpp"
#include "sample_cache/cache_storage_parser.hpp"
#include "sample_cache/metadata_storage.hpp"
#include "sample_cache/sample_type.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <timemory/utility/demangle.hpp>

namespace rocprofsys
{
namespace sample_cache
{
namespace
{
inline rocpd::data_processor&
get_data_processor()
{
    return rocpd::data_processor::get_instance();
}
template <typename CorrelationIdType>
inline uint64_t
get_parent_stack_id(const CorrelationIdType& correlation_id)
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
auto
get_kernel_dispatch_bulk_insert_executor()
{
    rocpd::data_storage::queries::table_insert_query query_builder;
    auto                                             query =
        query_builder
            .set_table_name("rocpd_kernel_dispatch_" +
                            rocpd::data_storage::database::get_instance().get_upid())
            .set_columns("guid", "nid", "pid", "tid", "agent_id", "kernel_id",
                         "dispatch_id", "queue_id", "stream_id", "start", "end",
                         "private_segment_size", "group_segment_size", "workgroup_size_x",
                         "workgroup_size_y", "workgroup_size_z", "grid_size_x",
                         "grid_size_y", "grid_size_z", "region_name_id", "event_id",
                         "extdata")
            .set_bulk_values(50, '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',
                             '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?')
            .get_query_string();

    return rocpd::data_storage::database::get_instance()
        .create_bulk_statement_executor<
            const char*, size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
            uint64_t, uint64_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
            size_t, size_t, size_t, const char*>(query);
}
}  // namespace

postprocessing_callback
rocpd_post_processing::get_kernel_dispatch_callback()
{
    m_kds_list.reserve(50);

    return [&](const storage_parsed_type_base& parsed) {
        const auto _kds = static_cast<const struct kernel_dispatch_sample&>(parsed);

        const auto  guid = rocpd::data_storage::database::get_instance().get_upid();
        auto&       data_processor = get_data_processor();
        auto&       agent_manager  = agent_manager::get_instance();
        const auto& n_info         = node_info::get_instance();
        const auto  process        = m_metadata.get_process_info();
        const auto  agent_primary_key =
            agent_manager.get_agent_by_handle(_kds.record.dispatch_info.agent_id.handle)
                .base_id;

        const auto thread_primary_key =
            data_processor.map_thread_id_to_primary_key(_kds.record.thread_id);

        const auto category_id = data_processor.insert_string(
            trait::name<category::rocm_kernel_dispatch>::value);

        const auto kernel_symbol =
            m_metadata.get_kernel_symbol(_kds.record.dispatch_info.kernel_id);

        if(!kernel_symbol.has_value())
        {
            throw std::runtime_error("Kernel symbol is missing for kernel dispatch");
            return;
        }

        const auto region_name_primary_key = data_processor.insert_string(
            tim::demangle(kernel_symbol->kernel_name).c_str());

        const auto stack_id        = _kds.record.correlation_id.internal;
        const auto parent_stack_id = get_parent_stack_id(_kds.record.correlation_id);
        const auto correlation_id  = 0;

        const auto event_id = data_processor.insert_event(
            category_id, stack_id, parent_stack_id, correlation_id);

        kernel_dispatch_insert kd(
            n_info.id, process.pid, thread_primary_key, agent_primary_key,
            _kds.record.dispatch_info.kernel_id, _kds.record.dispatch_info.dispatch_id,
            _kds.record.dispatch_info.queue_id.handle, _kds.stream_handle,
            _kds.record.start_timestamp, _kds.record.end_timestamp,
            _kds.record.dispatch_info.private_segment_size,
            _kds.record.dispatch_info.group_segment_size,
            _kds.record.dispatch_info.workgroup_size.x,
            _kds.record.dispatch_info.workgroup_size.y,
            _kds.record.dispatch_info.workgroup_size.z,
            _kds.record.dispatch_info.grid_size.x, _kds.record.dispatch_info.grid_size.y,
            _kds.record.dispatch_info.grid_size.z, region_name_primary_key, event_id);
        m_kds_list.emplace_back(kd);

        if(m_kds_list.size() == 50)
        {
            for(auto& kds : m_kds_list)
            {
                m_kernel_dispatch_bulk_insert_executor->bind_row_values(
                    guid.c_str(), kds.nid, kds.pid, kds.tid, kds.agent_id, kds.kernel_id,
                    kds.dispatch_id, kds.queue_id, kds.stream_id, kds.start, kds.end,
                    kds.private_segment_size, kds.group_segment_size,
                    kds.workgroup_size_x, kds.workgroup_size_y, kds.workgroup_size_z,
                    kds.grid_size_x, kds.grid_size_y, kds.grid_size_z, kds.region_name_id,
                    kds.event_id, "{}");
            }

            rocpd::data_storage::database::get_instance().execute_query(
                "BEGIN TRANSACTION;");
            m_kernel_dispatch_bulk_insert_executor->execute_bulk_insert();
            rocpd::data_storage::database::get_instance().execute_query("COMMIT;");
            m_kds_list.clear();
            m_kds_list.reserve(50);
        }
    };
}

postprocessing_callback
rocpd_post_processing::get_memory_copy_callback() const
{
    return [&](const storage_parsed_type_base& parsed) {
        const auto _mcs = static_cast<const struct memory_copy_sample&>(parsed);

        auto&       data_processor = get_data_processor();
        auto&       agent_manager  = agent_manager::get_instance();
        const auto& n_info         = node_info::get_instance();
        const auto  process        = m_metadata.get_process_info();

        const auto _name            = std::string{ m_metadata.get_buffer_name_info().at(
            _mcs.record.kind, _mcs.record.operation) };
        const auto name_primary_key = data_processor.insert_string(_name.c_str());

        const auto category_primary_key =
            data_processor.insert_string(trait::name<category::rocm_memory_copy>::value);

        const auto thread_primary_key =
            data_processor.map_thread_id_to_primary_key(_mcs.record.thread_id);

        const auto dst_agent_primary_key =
            agent_manager.get_agent_by_handle(_mcs.record.dst_agent_id.handle).base_id;
        const auto src_agent_primary_key =
            agent_manager.get_agent_by_handle(_mcs.record.src_agent_id.handle).base_id;

        const auto stack_id        = _mcs.record.correlation_id.internal;
        const auto parent_stack_id = get_parent_stack_id(_mcs.record.correlation_id);
        const auto correlation_id  = 0;
        const auto queue_id        = 0;

        const auto event_primary_key = data_processor.insert_event(
            category_primary_key, stack_id, parent_stack_id, correlation_id);

        data_processor.insert_memory_copy(
            n_info.id, process.pid, thread_primary_key, _mcs.record.start_timestamp,
            _mcs.record.end_timestamp, name_primary_key, dst_agent_primary_key,
            _mcs.record.dst_address.value, src_agent_primary_key,
            _mcs.record.src_address.value, _mcs.record.bytes, queue_id,
            _mcs.stream_handle, name_primary_key, event_primary_key);
    };
}

postprocessing_callback
rocpd_post_processing::get_memory_allocate_callback() const
{
    auto memtype_to_db =
        [](std::string_view memory_type) -> std::pair<std::string, std::string> {
        constexpr auto MEMORY_PREFIX  = std::string_view{ "MEMORY_ALLOCATION_" };
        constexpr auto SCRATCH_PREFIX = std::string_view{ "SCRATCH_MEMORY_" };
        constexpr auto VMEM_PREFIX    = std::string_view{ "VMEM_" };
        constexpr auto ASYNC_PREFIX   = std::string_view{ "ASYNC_" };

        std::string _type;
        std::string _level;

        if(memory_type.size() >= MEMORY_PREFIX.size() &&
           memory_type.substr(0, MEMORY_PREFIX.size()) == MEMORY_PREFIX)
        {
            _type = memory_type.substr(MEMORY_PREFIX.size());
            if(_type.size() >= VMEM_PREFIX.size() &&
               _type.substr(0, VMEM_PREFIX.size()) == VMEM_PREFIX)
            {
                _type  = _type.substr(VMEM_PREFIX.size());
                _level = "VIRTUAL";
            }
            else
            {
                _level = "REAL";
            }
        }
        else if(memory_type.size() >= SCRATCH_PREFIX.size() &&
                memory_type.substr(0, SCRATCH_PREFIX.size()) == SCRATCH_PREFIX)
        {
            _type  = memory_type.substr(SCRATCH_PREFIX.size());
            _level = "SCRATCH";
            if(_type.size() >= ASYNC_PREFIX.size() &&
               _type.substr(0, ASYNC_PREFIX.size()) == ASYNC_PREFIX)
            {
                _type = _type.substr(ASYNC_PREFIX.size());
            }
        }

        if(_type == "ALLOCATE")
        {
            _type = "ALLOC";
        }

        return std::make_pair(std::move(_type), std::move(_level));
    };

    return [&](const storage_parsed_type_base& parsed) {
        const auto  _mas = static_cast<const struct memory_allocate_sample&>(parsed);
        auto&       data_processor = get_data_processor();
        auto&       agent_manager  = agent_manager::get_instance();
        const auto& n_info         = node_info::get_instance();
        const auto  process        = m_metadata.get_process_info();
        const auto  thread_primary_key =
            data_processor.map_thread_id_to_primary_key(_mas.record.thread_id);
        auto agent_primary_key = std::optional<uint64_t>{};
        if(_mas.record.agent_id.handle != std::numeric_limits<uint64_t>::max())
        {
            agent_primary_key =
                agent_manager.get_agent_by_handle(_mas.record.agent_id.handle).base_id;
        }
        const auto* _name =
            m_metadata.get_buffer_name_info().at(_mas.record.kind, _mas.record.operation);

        const auto [type, level] = memtype_to_db(_name);

        const auto stack_id        = _mas.record.correlation_id.internal;
        const auto parent_stack_id = get_parent_stack_id(_mas.record.correlation_id);
        const auto correlation_id  = 0;
        const auto queue_id        = 0;

        const auto category_primary_key = data_processor.insert_string(
            trait::name<category::rocm_memory_allocate>::value);

        const auto event_primary_key = data_processor.insert_event(
            category_primary_key, stack_id, parent_stack_id, correlation_id);

        data_processor.insert_memory_alloc(
            n_info.id, process.pid, thread_primary_key, agent_primary_key, type.c_str(),
            level.c_str(), _mas.record.start_timestamp, _mas.record.end_timestamp,
            _mas.record.address.value, _mas.record.allocation_size, queue_id,
            _mas.stream_handle, event_primary_key);
    };
}

postprocessing_callback
rocpd_post_processing::get_region_callback() const
{
    auto parse_args = [](const std::string& arg_str) -> rocprofiler_sdk::function_args_t {
        rocprofiler_sdk::function_args_t args;

        if(arg_str.empty()) return args;

        constexpr std::string_view delimiter = ";;";

        const size_t                  estimated_tokens = (arg_str.length() / 10) + 1;
        std::vector<std::string_view> tokens;
        tokens.reserve(estimated_tokens);

        std::string_view sv(arg_str);
        size_t           start = 0;
        size_t           pos   = 0;

        while((pos = sv.find(delimiter, start)) != std::string_view::npos)
        {
            tokens.emplace_back(sv.substr(start, pos - start));
            start = pos + delimiter.length();
        }

        if(start < sv.length())
        {
            tokens.emplace_back(sv.substr(start));
        }

        if(tokens.size() % 4 != 0)
        {
            throw std::invalid_argument("Malformed argument string.");
        }

        // Reserve space for args
        args.reserve(tokens.size() / 4);

        // Process tokens in groups of 4
        for(size_t i = 0; i < tokens.size(); i += 4)
        {
            args.emplace_back(rocprofiler_sdk::argument_info{
                static_cast<uint32_t>(std::stoi(std::string(tokens[i]))),
                std::string(tokens[i + 1]), std::string(tokens[i + 2]),
                std::string(tokens[i + 3]) });
        }

        return args;
    };

    return [&](const storage_parsed_type_base& parsed) {
        const auto  _rs            = static_cast<const struct region_sample&>(parsed);
        auto&       data_processor = get_data_processor();
        const auto& n_info         = node_info::get_instance();
        const auto  process        = m_metadata.get_process_info();
        const auto  thread_primary_key =
            data_processor.map_thread_id_to_primary_key(_rs.record.thread_id);

        const auto callback_tracing_info = m_metadata.get_callback_tracing_info();
        const auto _name = std::string{ callback_tracing_info.at(_rs.record.kind,
                                                                 _rs.record.operation) };
        const auto name_primary_key = data_processor.insert_string(_name.c_str());

        const auto category_primary_key =
            data_processor.insert_string(_rs.category.c_str());

        const size_t stack_id        = _rs.record.correlation_id.internal;
        const size_t parent_stack_id = get_parent_stack_id(_rs.record.correlation_id);
        const size_t correlation_id  = 0;

        const auto event_primary_key =
            data_processor.insert_event(category_primary_key, stack_id, parent_stack_id,
                                        correlation_id, _rs.call_stack.c_str());

        const auto args = parse_args(_rs.args_str);
        for(const auto& arg : args)
        {
            data_processor.insert_args(event_primary_key, arg.arg_number,
                                       arg.arg_type.c_str(), arg.arg_name.c_str(),
                                       arg.arg_value.c_str());
        }

        data_processor.insert_region(n_info.id, process.pid, thread_primary_key,
                                     _rs.start_timestamp, _rs.end_timestamp,
                                     name_primary_key, event_primary_key);
    };
}
postprocessing_callback
rocpd_post_processing::get_in_time_sample_callback() const
{
    return [&](const storage_parsed_type_base& parsed) {
        const auto _its           = static_cast<const struct in_time_sample&>(parsed);
        auto&      data_processor = get_data_processor();
        const auto track_primary_key =
            data_processor.insert_string(_its.track_name.c_str());

        const auto event_id = data_processor.insert_event(
            track_primary_key, _its.stack_id, _its.parent_stack_id, _its.correlation_id,
            _its.call_stack.c_str(), _its.line_info.c_str(), _its.event_metadata.c_str());
        data_processor.insert_sample(_its.track_name.c_str(), _its.timestamp_ns, event_id,
                                     "{}");
    };
}
postprocessing_callback
rocpd_post_processing::get_pmc_event_with_sample_callback() const
{
    return [&](const storage_parsed_type_base& parsed) {
        const auto _pmc = static_cast<const struct pmc_event_with_sample&>(parsed);
        auto&      data_processor = get_data_processor();
        const auto track_primary_key =
            data_processor.insert_string(_pmc.track_name.c_str());

        auto&      agent_manager = agent_manager::get_instance();
        const auto agent_primary_key =
            agent_manager.get_agent_by_handle(_pmc.agent_handle).base_id;

        const auto event_id = data_processor.insert_event(
            track_primary_key, _pmc.stack_id, _pmc.parent_stack_id, _pmc.correlation_id,
            _pmc.call_stack.c_str(), _pmc.line_info.c_str(), _pmc.event_metadata.c_str());
        data_processor.insert_sample(_pmc.track_name.c_str(), _pmc.timestamp_ns, event_id,
                                     "{}");

        data_processor.insert_pmc_event(event_id, agent_primary_key,
                                        _pmc.pmc_info_name.c_str(), _pmc.value);
    };
}

rocpd_post_processing::rocpd_post_processing(metadata& md)
: m_metadata(md)
{}

void
rocpd_post_processing::register_parser_callback(storage_parser& parser)
{
    if(!get_use_rocpd())
    {
        return;
    }
    parser.register_type_callback(entry_type::region, get_region_callback());
    parser.register_type_callback(entry_type::kernel_dispatch,
                                  get_kernel_dispatch_callback());
    parser.register_type_callback(entry_type::memory_copy, get_memory_copy_callback());
    parser.register_type_callback(entry_type::memory_alloc,
                                  get_memory_allocate_callback());
    parser.register_type_callback(entry_type::in_time_sample,
                                  get_in_time_sample_callback());
    parser.register_type_callback(entry_type::pmc_event_with_sample,
                                  get_pmc_event_with_sample_callback());
};

void
rocpd_post_processing::post_process_metadata()
{
    if(!get_use_rocpd())
    {
        return;
    }
    auto& data_processor                   = get_data_processor();
    auto& agent_mngr                       = agent_manager::get_instance();
    auto  n_info                           = node_info::get_instance();
    m_kernel_dispatch_bulk_insert_executor = get_kernel_dispatch_bulk_insert_executor();

    data_processor.insert_node_info(n_info.id, n_info.hash, n_info.machine_id.c_str(),
                                    n_info.system_name.c_str(), n_info.node_name.c_str(),
                                    n_info.release.c_str(), n_info.version.c_str(),
                                    n_info.machine.c_str(), n_info.domain_name.c_str());

    auto process_info = m_metadata.get_process_info();
    data_processor.insert_process_info(n_info.id, process_info.ppid, process_info.pid, 0,
                                       0, 0, 0, process_info.command.c_str(), "{}");

    const auto& agents  = agent_mngr.get_agents();
    int         counter = 0;
    for(const auto& rocpd_agent : agents)
    {
        auto _base_id = rocpd::data_processor::get_instance().insert_agent(
            n_info.id, process_info.pid,
            ((rocpd_agent->type == agent_type::GPU) ? "GPU" : "CPU"), counter++,
            rocpd_agent->logical_node_id, rocpd_agent->logical_node_type_id,
            rocpd_agent->device_id, rocpd_agent->name.c_str(),
            rocpd_agent->model_name.c_str(), rocpd_agent->vendor_name.c_str(),
            rocpd_agent->product_name.c_str(), "");
        rocpd_agent->base_id = _base_id;
    }
    auto _string_list = m_metadata.get_string_list();
    for(auto& _string : _string_list)
    {
        data_processor.insert_string(std::string(_string).c_str());
    }

    auto _thread_info_list = m_metadata.get_thread_info_list();
    for(auto& t_info : _thread_info_list)
    {
        rocpd_insert_thread_id(t_info, n_info, process_info);
    }

    auto _track_info_list = m_metadata.get_track_info_list();
    for(auto& track : _track_info_list)
    {
        auto thread_id =
            track.thread_id.has_value()
                ? std::make_optional<size_t>(data_processor.map_thread_id_to_primary_key(
                      track.thread_id.value()))
                : std::nullopt;
        data_processor.insert_track(track.track_name.c_str(), n_info.id, process_info.pid,
                                    thread_id);
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

        data_processor.insert_string(kernel_name.c_str());
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

    auto buffer_info_list = m_metadata.get_buffer_name_info();
    for(const auto& buffer_info : buffer_info_list)
    {
        for(const auto& item : buffer_info.items())
        {
            data_processor.insert_string(*item.second);
        }
    }

    auto callback_info_list = m_metadata.get_callback_tracing_info();
    for(const auto& cb_info : callback_info_list)
    {
        for(const auto& item : cb_info.items())
        {
            data_processor.insert_string(*item.second);
        }
    }

    auto pmc_info_list = m_metadata.get_pmc_info_list();
    for(const auto& pmc_info : pmc_info_list)
    {
        const auto agent_primary_key =
            agent_mngr.get_agent_by_handle(pmc_info.agent_handle).base_id;

        data_processor.insert_pmc_description(
            n_info.id, process_info.pid, agent_primary_key, pmc_info.target_arch.c_str(),
            pmc_info.event_code, pmc_info.instance_id, pmc_info.name.c_str(),
            pmc_info.symbol.c_str(), pmc_info.description.c_str(),
            pmc_info.long_description.c_str(), pmc_info.component.c_str(),
            pmc_info.units.c_str(), pmc_info.value_type.c_str(), pmc_info.block.c_str(),
            pmc_info.expression.c_str(), pmc_info.is_constant, pmc_info.is_derived);
    }
};

inline void
rocpd_post_processing::rocpd_insert_thread_id(info::thread&        t_info,
                                              const node_info&     n_info,
                                              const info::process& process_info) const
{
    const auto& extended_info = thread_info::get(t_info.thread_id, SequentTID);
    if(extended_info.has_value())
    {
        t_info.start = extended_info->get_start();
        t_info.end   = extended_info->get_stop();
    }

    std::stringstream ss;
    ss << "Thread " << t_info.thread_id;
    get_data_processor().insert_thread_info(n_info.id, process_info.ppid,
                                            process_info.pid, t_info.thread_id,
                                            ss.str().c_str(), t_info.start, t_info.end);
}

void
rocpd_post_processing::flush_remaining()
{
    auto& data_processor = rocpd::data_processor::get_instance();
    if(!m_kds_list.empty())
    {
        for(const auto& kds : m_kds_list)
        {
            data_processor.insert_kernel_dispatch(
                kds.nid, kds.pid, kds.tid, kds.agent_id, kds.kernel_id, kds.dispatch_id,
                kds.queue_id, kds.stream_id, kds.start, kds.end, kds.private_segment_size,
                kds.group_segment_size, kds.workgroup_size_x, kds.workgroup_size_y,
                kds.workgroup_size_z, kds.grid_size_x, kds.grid_size_y, kds.grid_size_z,
                kds.region_name_id, kds.event_id, "{}");
        }
    }
}

}  // namespace sample_cache
}  // namespace rocprofsys
