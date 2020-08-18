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

#ifndef UDPDATASTRUCT_H
#define UDPDATASTRUCT_H

#include <qglobal.h>

#include <array>

#include <QString>

#include "vehicle-signals.h"

#include "QDataStream"

#include "QTextStream"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct udp_vehicle_data_t
{
    float       coord;

    float       velocity;

    int         direction;

    QString     DebugMsg;

    std::array<float, MAX_ANALOG_SIGNALS>   analogSignal;

    udp_vehicle_data_t()
        : coord(0.0f)
        , velocity(0.0f)
        , direction(1)
        , DebugMsg("")
    {
        std::fill(analogSignal.begin(), analogSignal.end(), 1.0f);
    }

    QByteArray serialize()
    {
        QByteArray result;

        QDataStream ds(&result, QIODevice::WriteOnly);

        ds << this->coord;

        ds << this->velocity;

        ds << this->direction;

        ds << this->DebugMsg;

        for (float &signal : this->analogSignal)
        {
            ds << signal;
        }

        return result;
    }

    void deserialize(QByteArray& array)
    {
        QDataStream ds(array);

        ds >> this->coord;

        ds >> this->velocity;

        ds >> this->direction;

        ds >> this->DebugMsg;

        for (float &signal : this->analogSignal)
        {
            ds >> signal;
        }
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

    unsigned int    msgCount;

    unsigned int    vehicleCount;

    QString         routeDir;

    QVector<udp_vehicle_data_t> vehicles;

    udp_server_data_t()
        : time(0.0f)
        , msgCount(1)
        , vehicleCount(1)
        , routeDir("")
    {
        udp_vehicle_data_t test;
        vehicles.append(test);
    }

    QByteArray serialize()
    {
        QByteArray result;

        QDataStream ds(&result, QIODevice::WriteOnly);

        ds << this->time;

        ds << this->msgCount;

        ds << this->vehicleCount;

        ds << this->routeDir;

        for (udp_vehicle_data_t &vehicle : this->vehicles)
        {
            ds << vehicle.serialize();
        }

        return result;
    }

    void deserialize(QByteArray& array)
    {
        QDataStream ds(array);

        ds >> this->time;

        ds >> this->msgCount;

        ds >> this->vehicleCount;

        ds >> this->routeDir;

        array.remove(0, sizeof (this->time));
        array.remove(0, sizeof (this->msgCount));
        array.remove(0, sizeof (this->vehicleCount));
        array.remove(0, sizeof (this->routeDir));

        for (udp_vehicle_data_t &vehicle : this->vehicles)
        {
            vehicle.deserialize(array);
            array.remove(0, sizeof (vehicle));
        }
    }
};

#endif // UDPDATASTRUCT_H
