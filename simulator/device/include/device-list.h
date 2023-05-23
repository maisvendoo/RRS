#ifndef     DEVICE_LIST_H
#define     DEVICE_LIST_H

#include    <QMetaType>
#include    <vector>
#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<Device *>   device_list_t;

Q_DECLARE_METATYPE(device_list_t)

#endif // DEVICE_LIST_H
