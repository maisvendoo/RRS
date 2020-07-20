//------------------------------------------------------------------------------
//
//
//
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief
 * \copyright
 * \author
 * \date
 */

#ifndef UPDDATASTRUCT_H
#define UPDDATASTRUCT_H

#include <qglobal.h>

#include "global-const.h"

#include <array>

#include <QString>

#include "vehicle-signals.h"


#include "QFile"
#include "QDataStream"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct udp_vehicle_data_t
{
    float       coord;
    float       velocity;
    QString     routePath;
    wchar_t     DebugMsg[DEBUG_STRING_SIZE];
    std::array<float, MAX_ANALOG_SIGNALS>   analogSignal;

    udp_vehicle_data_t()
        : coord(0.0f)
        , velocity(0.0f)
        , routePath("")
        , DebugMsg(L"")
    {
        std::fill(analogSignal.begin(), analogSignal.end(), 0.0f);
    }
};

/*!
 * \struct
 * \brief Trajectory element
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

struct udp_server_data_t
{
    float           time;
    unsigned long   msgCount;
    unsigned int    vehicleCount;

    QVector<udp_vehicle_data_t> vehicles;

    udp_server_data_t()
        : time(0.0f)
        , msgCount(0)
        , vehicleCount(0)
    {

    }
};

QByteArray serialize()
{
    return 0;
}

void desirialize(QByteArray array)
{

}

QFile file("");

QDataStream ss(&file);

#endif // UPDDATASTRUCT_H
