#ifndef     SYSTEM_DIAGNOSTIC_H
#define     SYSTEM_DIAGNOSTIC_H

#include    <gpu-info.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum StateGPU
{
    GPU_STATE_READY,
    GPU_STATE_VK_INSTANCE_ERROR,
    GPU_STATE_VK_ENUM_PHYSICAL_DEVICE_ERROR,
    GPU_STATE_NO_CAPABLE_DEVICES_ERROR,
    GPU_STATE_GET_DEVICES_LIST_ERROR
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
StateGPU getInfoGPUs(std::vector<gpu_info_t> &gpus_info);

#endif
