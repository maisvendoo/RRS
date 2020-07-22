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

    QByteArray serialize()
    {
        QByteArray result;

        QDataStream ds(&result, QIODevice::WriteOnly);

        ds << this->coord;

        ds << this->velocity;

        ds << this->routePath;

        ds << QString::fromWCharArray(this->DebugMsg);

        foreach (float signal, this->analogSignal)
        {
            ds << signal;
        }

        return result;
    }

    void deserialize(QByteArray& array)
    {
        QDataStream ds(array);

        ds >> this->coord;
        array.remove(0, sizeof (this->coord));

        ds >> this->velocity;
        array.remove(0, sizeof (this->velocity));

        ds >> this->routePath;
        array.remove(0, sizeof (this->routePath));

        QString str;
        ds >> str;
        str.toWCharArray(this->DebugMsg);
        array.remove(0, sizeof (str));

        foreach (float signal, &this->analogSignal)
        {
            ds >> signal;
            array.remove(0, sizeof (signal));
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

    QVector<udp_vehicle_data_t> vehicles;

    udp_server_data_t()
        : time(0.0f)
        , msgCount(0)
        , vehicleCount(0)
    {

    }

    QByteArray serialize()
    {
        QByteArray result;

        QDataStream ds(&result, QIODevice::WriteOnly);

        ds << this->time;

        ds << this->msgCount;

        ds << this->vehicleCount;

        foreach (udp_vehicle_data_t vehicle, this->vehicles)
        {
            ds << vehicle.serialize();
        }

        return result;
    }

    void deserialize(QByteArray& array)
    {
        QDataStream ds(array);

        ds >> this->time;

        array.remove(0, sizeof (this->time));

        ds >> this->msgCount;

        array.remove(0, sizeof (this->msgCount));

        ds >> this->vehicleCount;

        array.remove(0, sizeof (this->vehicleCount));

        foreach (udp_vehicle_data_t vehicle, &this->vehicles)
        {
            vehicle.deserialize(array);
        }
    }
};

#endif // UPDDATASTRUCT_H
