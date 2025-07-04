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

namespace rocprofsys
{
namespace cache
{
namespace metadata
{
void
storage::add_pmc_info(const pmc_info& pmc_info)
{
    m_pmc_infos.emplace(pmc_info);
}

void
storage::add_thread_info(const thread_info& thread_info)
{
    m_threads.emplace(thread_info);
}

std::optional<pmc_info>
storage::get_pmc_info(const std::string_view& unique_name)
{
    return m_pmc_infos.find(
        [&](const metadata::pmc_info& value) { return value.name == unique_name; });
}

std::optional<thread_info>
storage::get_thread_info(const uint32_t& thread_id)
{
    return m_threads.find(
        [&](const metadata::thread_info& value) { return value.thread_id == thread_id; });
}

}  // namespace metadata
}  // namespace cache
}  // namespace rocprofsys
