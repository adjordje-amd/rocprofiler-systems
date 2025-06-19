#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include <rocprofiler-sdk/agent.h>

#if ROCPROFSYS_USE_ROCM > 0
#    include <amd_smi/amdsmi.h>
#endif

namespace rocprofsys
{
namespace rocpd
{

struct agent
{
    const rocprofiler_agent_v0_t* agent = nullptr;
    size_t                        device_id {0};
    size_t                        base_id {0};

#if ROCPROFSYS_USE_ROCM > 0
    amdsmi_processor_handle       smi_handle = nullptr;
#endif

};

}  // namespace rocpd
}  // namespace rocprofsys
