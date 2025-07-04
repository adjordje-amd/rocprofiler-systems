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
template <typename T>
class synced_set
{
public:
    void emplace(const T& value)
    {
        std::lock_guard guard{ m_mutex };
        m_set.emplace(value);
    }

    std::optional<T> find(std::function<bool(const T&)> predicate)
    {
        std::lock_guard guard{ m_mutex };
        auto            it = std::find_if(m_set.begin(), m_set.end(), predicate);
        if(it == m_set.end())
        {
            return std::nullopt;
        }
        return *it;
    }

private:
    std::mutex  m_mutex;
    std::set<T> m_set;
};

struct pmc_info
{
    uint32_t    agent_abs_index;
    std::string target_arch;
    uint32_t    event_code;
    uint32_t    instance_id;
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
    uint32_t    parent_process_id;
    uint32_t    process_id;
    uint32_t    thread_id;
    std::string name;
    uint32_t    start;
    uint32_t    end;
    std::string extdata;
    friend bool operator<(const thread_info& lhs, const thread_info& rhs)
    {
        return lhs.thread_id < rhs.thread_id;
    }
};

struct storage
{
    void                    add_pmc_info(const pmc_info& pmc_info);
    std::optional<pmc_info> get_pmc_info(const std::string_view& unique_name);

    void                       add_thread_info(const thread_info& thread_info);
    std::optional<thread_info> get_thread_info(const uint32_t& thread_id);

private:
    synced_set<pmc_info>    m_pmc_infos;
    synced_set<thread_info> m_threads;
};

}  // namespace metadata
}  // namespace cache
}  // namespace rocprofsys
