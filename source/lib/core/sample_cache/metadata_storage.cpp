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

#include "metadata_storage.hpp"
#include <rocprofiler-sdk/callback_tracing.h>

namespace rocprofsys
{
namespace cache
{
namespace metadata
{
storage&
storage::get_instance()
{
    static storage _instance;
    return _instance;
}

void
storage::set_process(const process_info& process)
{
    m_process = process;
}

void
storage::add_pmc_info(const pmc_info& pmc_info)
{
    m_pmc_infos.wlock([&pmc_info](auto& _data) {
        if(_data.count(pmc_info) > 0)
        {
            return;
        }
        _data.emplace(pmc_info);
    });
}

void
storage::add_thread_info(const thread_info& thread_info)
{
    m_threads.wlock([&thread_info](auto& _data) {
        if(_data.count(thread_info) > 0)
        {
            return;
        }
        _data.emplace(thread_info);
    });
}
void
storage::add_code_object(
    const rocprofiler_callback_tracing_code_object_load_data_t& code_object)
{
    m_code_objects.wlock([&code_object](auto& _data) {
        if(_data.count(code_object) > 0)
        {
            return;
        }
        _data.emplace(code_object);
    });
}

void
storage::add_kernel_symbol(
    const rocprofiler_callback_tracing_code_object_kernel_symbol_register_data_t&
        kernel_symbol)
{
    m_kernel_symbols.wlock([&kernel_symbol](auto& _data) {
        if(_data.count(kernel_symbol) > 0)
        {
            return;
        }
        _data.emplace(kernel_symbol);
    });
}

process_info
storage::get_process_info() const
{
    return m_process;
}

std::optional<pmc_info>
storage::get_pmc_info(const std::string_view& unique_name) const
{
    std::optional<pmc_info> result = std::nullopt;
    m_pmc_infos.rlock([&unique_name, &result](const auto& data) {
        auto it =
            std::find_if(data.begin(), data.end(), [&unique_name](const pmc_info& val) {
                return val.name == unique_name;
            });
        if(it == data.end())
        {
            result = std::nullopt;
            return;
        }
        result = *it;
    });
    return result;
}

std::optional<thread_info>
storage::get_thread_info(const uint32_t& thread_id) const
{
    std::optional<thread_info> result = std::nullopt;
    m_threads.rlock([&thread_id, &result](const auto& data) {
        auto it =
            std::find_if(data.begin(), data.end(), [&thread_id](const thread_info& val) {
                return val.thread_id == thread_id;
            });
        if(it == data.end())
        {
            result = std::nullopt;
            return;
        }
        result = *it;
    });
    return result;
}

std::optional<rocprofiler_callback_tracing_code_object_load_data_t>
storage::get_code_object(uint64_t code_object_id) const
{
    std::optional<rocprofiler_callback_tracing_code_object_load_data_t> result =
        std::nullopt;
    m_code_objects.rlock([&code_object_id, &result](const auto& data) {
        auto it = std::find_if(
            data.begin(), data.end(),
            [&code_object_id](
                const rocprofiler_callback_tracing_code_object_load_data_t& val) {
                return val.code_object_id == code_object_id;
            });
        if(it == data.end())
        {
            result = std::nullopt;
            return;
        }
        result = *it;
    });
    return result;
}

std::optional<rocprofiler_callback_tracing_code_object_kernel_symbol_register_data_t>
storage::get_kernel_symbol(uint64_t kernel_id) const
{
    std::optional<rocprofiler_callback_tracing_code_object_kernel_symbol_register_data_t>
        result = std::nullopt;
    m_kernel_symbols.rlock([&kernel_id, &result](const auto& data) {
        auto it = std::find_if(
            data.begin(), data.end(),
            [&kernel_id](
                const rocprofiler_callback_tracing_code_object_kernel_symbol_register_data_t&
                    val) { return val.kernel_id == kernel_id; });
        if(it == data.end())
        {
            result = std::nullopt;
            return;
        }
        result = *it;
    });
    return result;
}

}  // namespace metadata
}  // namespace cache
}  // namespace rocprofsys
