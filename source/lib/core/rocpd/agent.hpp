#pragma once

#include <cstddef>
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

    size_t      id;
    size_t      device_id;
    device_type type;
};

}  // namespace rocpd
}  // namespace rocprofsys