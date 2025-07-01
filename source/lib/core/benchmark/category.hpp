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

#include <string_view>

namespace rocprofiler
{
namespace benchmark
{
enum class category
{
    Kernel_Dispatch,
    DB_Entry_Kernel_Dispatch,
    Kernel_Dispatch_Cached,
    Memory_Copy,
    DB_Entry_Memory_Copy,
    Memory_Copy_Cached,
    Memory_Allocate,
    DB_Entry_Memory_Allocate,
    Memory_Allocate_Cached,
    Perfetto_Kernel_Dispatch,
    Sdk_Tool_Buffered_Tracing,
    Event,
    Event_Cached,
    Count
};

constexpr std::string_view
to_string(category cat)
{
    switch(cat)
    {
        case category::Kernel_Dispatch: return "Kernel_Dispatch";
        case category::Kernel_Dispatch_Cached: return "Kernel_Dispatch_Cached";
        case category::DB_Entry_Kernel_Dispatch: return "DB_Entry_Kernel_Dispatch";
        case category::Memory_Copy: return "Memory_Copy";
        case category::Memory_Copy_Cached: return "Memory_Copy_Cached";
        case category::Memory_Allocate: return "Memory_Allocate";
        case category::Memory_Allocate_Cached: return "Memory_Allocate_Cached";
        case category::DB_Entry_Memory_Copy: return "DB_Entry_Memory_Copy";
        case category::DB_Entry_Memory_Allocate: return "DB_Entry_Memory_Allocate";
        case category::Perfetto_Kernel_Dispatch: return "Perfetto_Kernel_Dispatch";
        case category::Sdk_Tool_Buffered_Tracing: return "Sdk_Tool_Buffered_Tracing";
        case category::Event: return "Event";
        case category::Event_Cached: return "Event_Cached";
        default: return "Unknown";
    }
}

}  // namespace benchmark
}  // namespace rocprofiler
