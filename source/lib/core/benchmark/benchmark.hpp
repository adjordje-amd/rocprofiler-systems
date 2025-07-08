// MIT License
//
// Copyright (c) 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
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

#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <mutex>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "core/benchmark/category.hpp"
#include "core/debug.hpp"

namespace rocprofiler
{
namespace benchmark
{
namespace
{
template <bool Enabled, typename CategoryEnum, CategoryEnum... EnabledCategories>
struct benchmark_impl
{
    template <CategoryEnum... Categories>
    struct scope
    {
        scope(const scope&) = delete;
        scope& operator=(const scope&) = delete;
        ~scope()                       = default;

    protected:
        scope()        = default;
        scope(scope&&) = default;
        scope& operator=(scope&&) = default;
    };

    template <CategoryEnum... Categories>
    static void start()
    {}

    template <CategoryEnum... Categories>
    static void end()
    {}

    template <CategoryEnum... Categories>
    [[nodiscard]] static scope<Categories...> scoped_trace()
    {
        return scope<Categories...>{};
    }

    static void init_from_env(const char* = nullptr) {}
    static void show_results() {}
};

template <typename CategoryEnum, CategoryEnum... EnabledCategories>
struct benchmark_impl<true, CategoryEnum, EnabledCategories...>
{
    static_assert(std::is_enum_v<CategoryEnum>, "CategoryEnum must be an enum");

public:
    using clock                            = std::chrono::high_resolution_clock;
    using time_point                       = clock::time_point;
    static constexpr size_t kMaxCategories = static_cast<size_t>(CategoryEnum::Count);

    template <CategoryEnum... Categories>
    struct scope
    {
        friend benchmark_impl;

    public:
        scope(const scope&) = delete;
        scope& operator=(const scope&) = delete;
        ~scope() { end<Categories...>(); }

    protected:
        scope() { start<Categories...>(); }

        scope(scope&&) = default;
        scope& operator=(scope&&) = default;
    };

    template <CategoryEnum... Categories>
    static void start()
    {
        const auto      now = clock::now();
        std::lock_guard lock(mutex_);
        (..., (if_compiled<Categories>([&] {
             if(runtimeEnabled_.test(to_index(Categories)))
                 startTimes_[to_index(Categories)] = now;
         })));
    }

    template <CategoryEnum... Categories>
    static void end()
    {
        const auto      endTime = clock::now();
        std::lock_guard lock(mutex_);
        (..., (if_compiled<Categories>([&] {
             if(runtimeEnabled_.test(to_index(Categories)))
                 endCategory(endTime, Categories);
         })));
    }

    template <CategoryEnum... Categories>
    [[nodiscard]] static scope<Categories...> scoped_trace()
    {
        return scope<Categories...>{};
    }

    static void init_from_env(const char* envVar = "BENCHMARK_CATEGORIES")
    {
        std::lock_guard lock(mutex_);
        const auto*     env = std::getenv(envVar);
        if(env == nullptr || std::string(env).empty())
        {
            ROCPROFSYS_WARNING(
                1, "No BENCHMARK categories specified in environment variable.\n");
            return;
        }
        std::string        str(env);
        std::istringstream ss(str);
        std::string        token;

        while(std::getline(ss, token, ','))
        {
            token.erase(0, token.find_first_not_of(" \t"));
            token.erase(token.find_last_not_of(" \t") + 1);
            for(CategoryEnum cat : compiledCategories)
            {
                if(to_string(cat) == token)
                {
                    runtimeEnabled_.set(to_index(cat));
                }
            }
        }
    }

    static void show_results()
    {
        std::lock_guard                                   lock(mutex_);
        std::vector<std::pair<CategoryEnum, result_data>> sorted;

        for(CategoryEnum cat : compiledCategories)
        {
            const auto& data = results_[to_index(cat)];
            if(data.count > 0)
            {
                sorted.emplace_back(cat, data);
            }
        }

        std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
            return a.second.totalTime > b.second.totalTime;
        });

        constexpr uint32_t wCategory = 30;
        constexpr uint32_t wCalls    = 8;
        constexpr uint32_t wTotal    = 12;
        constexpr uint32_t wAvg      = 10;
        constexpr uint32_t wMin      = 10;
        constexpr uint32_t wMax      = 10;

        std::cout << "\033[32m"
                  << std::string(wCategory + wCalls + wTotal + wAvg + wMin + wMax, '=')
                  << "\n";
        std::cout << "Benchmark Results (Sorted by Total Time):\n";
        std::cout << std::string(wCategory + wCalls + wTotal + wAvg + wMin + wMax, '-')
                  << "\n";
        std::cout << std::left << std::setw(wCategory) << "Category" << std::right
                  << std::setw(wCalls) << "Calls" << std::setw(wTotal) << "Total(ms)"
                  << std::setw(wAvg) << "Avg(us)" << std::setw(wMin) << "Min(us)"
                  << std::setw(wMax) << "Max(us)"
                  << "\n";

        std::cout << std::string(wCategory + wCalls + wTotal + wAvg + wMin + wMax, '-')
                  << "\n";

        for(const auto& [cat, data] : sorted)
        {
            double totalMs = static_cast<double>(data.totalTime) / 1000.0;
            double avgUs   = static_cast<double>(data.totalTime) / data.count;

            std::cout << std::left << std::setw(wCategory) << to_string(cat) << std::right
                      << std::setw(wCalls) << data.count << std::setw(wTotal)
                      << std::fixed << std::setprecision(3) << totalMs << std::setw(wAvg)
                      << std::fixed << std::setprecision(1) << avgUs << std::setw(wMin)
                      << data.minTime << std::setw(wMax) << data.maxTime << "\n";
        }

        std::cout << std::string(wCategory + wCalls + wTotal + wAvg + wMin + wMax, '=')
                  << "\033[0m"
                  << "\n\n";
    }

private:
    struct result_data
    {
        uint64_t totalTime = 0;
        size_t   count     = 0;
        uint64_t minTime   = std::numeric_limits<uint64_t>::max();
        uint64_t maxTime   = std::numeric_limits<uint64_t>::min();

        void update(uint64_t duration)
        {
            totalTime += duration;
            count += 1;
            if(duration < minTime) minTime = duration;
            if(duration > maxTime) maxTime = duration;
        }
    };

    static constexpr size_t to_index(CategoryEnum cat)
    {
        return static_cast<size_t>(cat);
    }

    static void endCategory(const time_point& endTime, CategoryEnum cat)
    {
        const size_t idx = to_index(cat);
        auto         it  = startTimes_.find(idx);
        if(it == startTimes_.end())
        {
            ROCPROFSYS_WARNING(1, "Benchmark error: missing start time for category!\n");
            return;
        }

        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(endTime - it->second)
                .count();
        startTimes_.erase(it);
        results_[idx].update(duration);
    }

    template <CategoryEnum Cat, typename Func>
    static constexpr void if_compiled(Func&& f)
    {
        if constexpr(((Cat == EnabledCategories) || ...))
        {
            f();
        }
    }

    static constexpr std::array<CategoryEnum, sizeof...(EnabledCategories)>
        compiledCategories = { EnabledCategories... };

    static inline std::unordered_map<size_t, time_point>  startTimes_;
    static inline std::array<result_data, kMaxCategories> results_{};
    static inline std::bitset<kMaxCategories>             runtimeEnabled_;
    static inline std::mutex                              mutex_;
};

#ifdef ROCPROFSYS_ENABLE_BENCHMARK
using rps_benchmark = benchmark::benchmark_impl<
    static_cast<bool>(ROCPROFSYS_ENABLE_BENCHMARK), benchmark::category,
    benchmark::category::Kernel_Dispatch, benchmark::category::Memory_Copy,
    benchmark::category::Memory_Allocate, benchmark::category::DB_Entry_Kernel_Dispatch,
    benchmark::category::DB_Entry_Memory_Copy,
    benchmark::category::DB_Entry_Memory_Allocate,
    benchmark::category::Perfetto_Kernel_Dispatch,
    benchmark::category::Sdk_Tool_Buffered_Tracing>;
#else
using rps_benchmark = benchmark::benchmark_impl<false, benchmark::category>;
#endif
}  // namespace

template <category... Categories>
void
start()
{
    rps_benchmark::template start<Categories...>();
}

template <category... Categories>
void
end()
{
    rps_benchmark::template end<Categories...>();
}

template <category... Categories>
[[nodiscard]] auto
scoped_trace()
{
    return rps_benchmark::template scoped_trace<Categories...>();
}

inline void
init_from_env(const char* envVar = "BENCHMARK_CATEGORIES")
{
    rps_benchmark::init_from_env(envVar);
}

inline void
show_results()
{
    rps_benchmark::show_results();
}

}  // namespace benchmark
}  // namespace rocprofiler
