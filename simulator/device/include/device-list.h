#ifndef     DEVICE_LIST_H
#define     DEVICE_LIST_H

#include    <vector>
#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<Device *>   device_list_t;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct device_coord_t
{
    Device *device = nullptr;
    double coord = 0.0;

    device_coord_t()
    {

    }
};

typedef std::vector<device_coord_t>   device_coord_list_t;

#endif // DEVICE_LIST_H
