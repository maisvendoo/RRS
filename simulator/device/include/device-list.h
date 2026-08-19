#ifndef     DEVICE_LIST_H
#define     DEVICE_LIST_H

#include    <vector>

class       Device;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using device_list_t = std::vector<Device*>;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct device_coord_t
{
    Device* device{nullptr};
    double coord{0.0};
};

using device_coord_list_t = std::vector<device_coord_t>;

#endif // DEVICE_LIST_H
