#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace rocprofsys
{
namespace rocpd
{

struct agent
{
    enum class device_type
    {
        cpu = 1,
        gpu = 2
    };

    uint64_t    handle;
    size_t      device_id;
    device_type type;
};

}  // namespace rocpd
}  // namespace rocprofsys
