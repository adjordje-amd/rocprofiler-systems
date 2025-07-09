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
        case category::Memory_Allocate_Cached: return "Memory_Allocate_Cache";
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
