#pragma once

#include <string>

namespace benchmark
{
  enum class category
  {
    Kernel_Dispatch,
    DB_Entry_Kernel_Dispatch,
    Memory_Copy,
    DB_Entry_Memory_Copy,
    Memory_Allocate,
    DB_Entry_Memory_Allocate,
    Count
  };

  constexpr std::string_view to_string(category cat)
  {
    switch(cat)
    {
      case category::Kernel_Dispatch: return "Kernel_Dispatch";
      case category::DB_Entry_Kernel_Dispatch: return "DB_Entry_Kernel_Dispatch";
      case category::Memory_Copy: return "Memory_Copy";
      case category::Memory_Allocate: return "Memory_Allocate";
      case category::DB_Entry_Memory_Copy: return "DB_Entry_Memory_Copy";
      case category::DB_Entry_Memory_Allocate: return "DB_Entry_Memory_Allocate";
      default: return "Unknown";
    }
  }

} // namespace benchmark
