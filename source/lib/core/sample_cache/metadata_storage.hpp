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

#include "common/synchronized.hpp"
#include "library/thread_info.hpp"
#include "sample_type.hpp"
#include <array>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <rocprofiler-sdk/callback_tracing.h>
#include <set>
#include <stdint.h>
#include <string.h>
#include <string>
#include <type_traits>

namespace rocprofsys
{
namespace cache
{
namespace metadata
{
struct process_info
{
    int         pid;
    std::string command;
};
struct pmc_info
{
    size_t      agent_abs_index;
    std::string target_arch;
    size_t      event_code;
    size_t      instance_id;
    std::string name;
    std::string symbol;
    std::string description;
    std::string long_description;
    std::string component;
    std::string units;
    std::string value_type;
    std::string block;
    std::string expression;
    uint32_t    is_constant;
    uint32_t    is_derived;
    std::string extdata;

    friend bool operator<(const pmc_info& lhs, const pmc_info& rhs)
    {
        return lhs.name.compare(rhs.name) < 0;
    }
};

struct thread_info
{
    int32_t     parent_process_id;
    int32_t     process_id;
    uint64_t    thread_id;
    uint32_t    start;
    uint32_t    end;
    std::string extdata;
    friend bool operator<(const thread_info& lhs, const thread_info& rhs)
    {
        return lhs.thread_id < rhs.thread_id;
    }
};

struct code_object_less
{
    bool operator()(const rocprofiler_callback_tracing_code_object_load_data_t& lhs,
                    const rocprofiler_callback_tracing_code_object_load_data_t& rhs) const
    {
        return lhs.code_object_id < rhs.code_object_id;
    }
};

struct kernel_symbol_less
{
    bool operator()(
        const rocprofiler_callback_tracing_code_object_kernel_symbol_register_data_t& lhs,
        const rocprofiler_callback_tracing_code_object_kernel_symbol_register_data_t& rhs)
        const
    {
        return lhs.kernel_object < rhs.kernel_object;
    }
};

struct storage
{
    static storage& get_instance();

    void         set_process(const process_info& process);
    process_info get_process_info() const;

    void                    add_pmc_info(const pmc_info& pmc_info);
    std::optional<pmc_info> get_pmc_info(const std::string_view& unique_name) const;

    void                       add_thread_info(const thread_info& thread_info);
    std::optional<thread_info> get_thread_info(const uint32_t& thread_id) const;

    void add_code_object(
        const rocprofiler_callback_tracing_code_object_load_data_t& code_object);
    std::optional<rocprofiler_callback_tracing_code_object_load_data_t> get_code_object(
        uint64_t code_object_id) const;

    void add_kernel_symbol(
        const rocprofiler_callback_tracing_code_object_kernel_symbol_register_data_t&
            kernel_symbol);
    std::optional<rocprofiler_callback_tracing_code_object_kernel_symbol_register_data_t>
    get_kernel_symbol(uint64_t kernel_id) const;

    void print_pmc_info() const
    {
        m_pmc_infos.rlock([](auto& _pmcs) {
            std::cout << "Printing PMCs:\n";
            for(const auto& pmc : _pmcs)
            {
                std::cout << pmc.symbol << "\n";
            }
            std::cout << std::endl;
        });
    }

    void print_threads() const
    {
        m_threads.rlock([](auto& _set) {
            std::cout << "Printing Threads:\n";
            for(const auto& thread : _set)
            {
                std::cout << thread.thread_id << "\n";
            }
            std::cout << std::endl;
        });
    }

    void print_code_objects() const
    {
        m_code_objects.rlock([](auto& _set) {
            std::cout << "Printing CodeObjects:\n";
            for(const auto& code_object : _set)
            {
                std::cout << code_object.uri << "\n";
            }
            std::cout << std::endl;
        });
    }

    void print_kernel_symbols() const
    {
        m_kernel_symbols.rlock([](auto& _set) {
            std::cout << "Printing KernelSymbols:\n";
            for(const auto& kernel_symbol : _set)
            {
                std::cout << kernel_symbol.kernel_id << "\n";
            }
            std::cout << std::endl;
        });
    }

private:
    storage() = default;
    process_info                                m_process;
    common::synchronized<std::set<pmc_info>>    m_pmc_infos;
    common::synchronized<std::set<thread_info>> m_threads;
    common::synchronized<
        std::set<rocprofiler_callback_tracing_code_object_load_data_t, code_object_less>>
        m_code_objects;
    common::synchronized<
        std::set<rocprofiler_callback_tracing_code_object_kernel_symbol_register_data_t,
                 kernel_symbol_less>>
        m_kernel_symbols;
};

}  // namespace metadata
}  // namespace cache
}  // namespace rocprofsys
