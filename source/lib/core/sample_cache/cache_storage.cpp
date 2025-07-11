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

#include "cache_storage.hpp"
#include "library/ptl.hpp"
#include <mutex>

namespace rocprofsys
{
namespace cache
{
storage&
storage::get_instance()
{
    static storage _instance;
    return _instance;
}

storage::storage()
{
    tasking::general::get_task_group().exec([this]() {
        std::filesystem::path path{ filename };
        std::ofstream         ofs(path, std::ios::binary | std::ios::out);

        if(!ofs)
        {
            std::cerr << "Error opening file for writing: " << path << "\n";
            return;
        }

        auto execute_flush = [&](std::ofstream& ofs, bool force = false) {
            size_t head, tail;
            {
                std::lock_guard guard{ m_mutex };
                head = m_head;
                tail = m_tail;

                if(head == tail)
                {
                    return;
                }

                auto used_space =
                    m_head > m_tail ? (m_head - m_tail) : (buffer_size - m_tail + m_head);
                if(!force && used_space < flush_treshhold)
                {
                    return;
                }
                m_tail = m_head;
            }

            if(head > tail)
            {
                ofs.write(reinterpret_cast<const char*>(m_buffer->data() + tail),
                          head - tail);
            }
            else
            {
                ofs.write(reinterpret_cast<const char*>(m_buffer->data() + tail),
                          buffer_size - tail);
                ofs.write(reinterpret_cast<const char*>(m_buffer->data()), head);
            }
        };

        std::mutex shutdown_condition_mutex;
        while(!m_shutdown)
        {
            execute_flush(ofs);
            std::unique_lock lock{ shutdown_condition_mutex };
            m_shutdown_condition.wait_for(lock, std::chrono::milliseconds(30),
                                          [&]() { return m_shutdown; });
        }

        execute_flush(ofs, true);
        ofs.close();
        m_exit_condition.notify_one();
    });
}

void
storage::shutdown()
{
    m_shutdown = true;
    m_shutdown_condition.notify_all();
    std::mutex       exit_mutex;
    std::unique_lock exit_lock{ exit_mutex };
    m_exit_condition.wait(exit_lock);
}

void
storage::fragment_memory()
{
    auto* data = m_buffer->data();
    memset(data + m_head, 0xFFFF, buffer_size - m_head);
    *reinterpret_cast<sample_type*>(data + m_head) = sample_type::fragmented_space;

    size_t remining_bytes = buffer_size - m_head - minimal_fragmented_memory_size;
    *reinterpret_cast<size_t*>(data + m_head + sizeof(sample_type)) = remining_bytes;
    m_head                                                          = 0;
}

uint8_t*
storage::reserve_memory_space(size_t len)
{
    size_t size;
    {
        std::lock_guard scope{ m_mutex };

        if((m_head + len + minimal_fragmented_memory_size) > buffer_size)
        {
            fragment_memory();
        }
        size   = m_head;
        m_head = m_head + len;
    }

    auto* result = m_buffer->data() + size;
    memset(result, 0, len);
    return result;
};

}  // namespace cache
}  // namespace rocprofsys
