#ifndef     SYSTEM_DIAGNOSTIC_H
#define     SYSTEM_DIAGNOSTIC_H

#include    "winver.h"
#include    <gpu-info.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum StateGPU
{
    GPU_STATE_READY,
    GPU_STATE_VULKAN_LOADER_NOT_FOUND_ERROR,
    GPU_STATE_VK_INSTANCE_ERROR,
    GPU_STATE_VK_ENUM_PHYSICAL_DEVICE_ERROR,
    GPU_STATE_NO_CAPABLE_DEVICES_ERROR,
    GPU_STATE_GET_DEVICES_LIST_ERROR
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum VendorGPU
{
    VID_NVIDIA = 0x10DE,
    VID_AMD = 0x1002,
    VID_INTEL = 0x8086
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
StateGPU getInfoGPUs(std::vector<gpu_info_t> &gpus_info);

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool checkOperationSystemVersion(uint32_t vendorID, const RequireWindowsVersion &winver, QString &productName);

#endif
