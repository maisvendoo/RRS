#ifndef     GPU_INFO_H
#define     GPU_INFO_H

#include    <QString>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct gpu_info_t
{
    QString     deviceName = "";
    QString     deviceType = "";
    uint32_t    vendorID = 0;
    uint32_t    deviceID = 0;
};

#endif
