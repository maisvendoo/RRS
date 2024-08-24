#ifndef     SIMULATOR_UPDATE_STRUCT_H
#define     SIMULATOR_UPDATE_STRUCT_H

#include    "global-const.h"

#include    "vehicle-signals.h"

#include    <array>

#include    <QString>
#include    <QByteArray>
#include    <QBuffer>
#include    <QDataStream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_vehicle_update_t
{
    double  position_x;
    double  position_y;
    double  position_z;
    double  orth_x;
    double  orth_y;
    double  orth_z;
    double  up_x;
    double  up_y;
    double  up_z;
    int     orientation;
    int     prev_vehicle;
    int     next_vehicle;
    double  length;
    std::array<float, MAX_ANALOG_SIGNALS>   analogSignal;

    simulator_vehicle_update_t()
        : position_x(0.0), position_y(0.0), position_z(0.0)
        , orth_x(0.0), orth_y(0.0), orth_z(0.0)
        , up_x(0.0), up_y(0.0), up_z(1.0)
        , orientation(1)
        , prev_vehicle(-1)
        , next_vehicle(-1)
        , length(0.0)
    {
        std::fill(analogSignal.begin(), analogSignal.end(), 0.0f);
    }

    QByteArray serialize()
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << position_x;
        stream << position_y;
        stream << position_z;
        stream << orth_x;
        stream << orth_y;
        stream << orth_z;
        stream << up_x;
        stream << up_y;
        stream << up_z;
        stream << orientation;
        stream << prev_vehicle;
        stream << next_vehicle;
        stream << length;

        for (auto signal : analogSignal)
        {
            stream << signal;
        }

        return buff.data();
    }

    void deserialize(QByteArray &data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> position_x;
        stream >> position_y;
        stream >> position_z;
        stream >> orth_x;
        stream >> orth_y;
        stream >> orth_z;
        stream >> up_x;
        stream >> up_y;
        stream >> up_z;
        stream >> orientation;
        stream >> prev_vehicle;
        stream >> next_vehicle;
        stream >> length;

        for (size_t i = 0; i < analogSignal.size(); ++i)
        {
            stream >> analogSignal[i];
        }
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_update_t
{
    double  time;
    int     current_vehicle;
    wchar_t currentDebugMsg[DEBUG_STRING_SIZE];
    int     controlled_vehicle;
    wchar_t controlledDebugMsg[DEBUG_STRING_SIZE];
    std::array<simulator_vehicle_update_t, MAX_NUM_VEHICLES>  vehicles;

    simulator_update_t()
        : time(0.0)
        , current_vehicle(0)
        , currentDebugMsg(L"")
        , controlled_vehicle(0)
        , controlledDebugMsg(L"")
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct tcp_simulator_update_t
{
    double time = 0;
    int current_vehicle = 0;
    QString currentDebugMsg = "";
    int controlled_vehicle = 0;
    QString controlledDebugMeg = "";
    std::vector<simulator_vehicle_update_t> vehicles;

    QByteArray serialize()
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << time;
        stream << current_vehicle;
        stream << currentDebugMsg;
        stream << controlled_vehicle;
        stream << controlledDebugMeg;

        stream << vehicles.size();

        for (auto vehicle : vehicles)
        {
            stream << vehicle.serialize();
        }

        return buff.data();
    }

    void deserialize(QByteArray &data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> time;
        stream >> current_vehicle;
        stream >> currentDebugMsg;
        stream >> controlled_vehicle;
        stream >> controlledDebugMeg;

        size_t num_vehicles;

        stream >> num_vehicles;

        vehicles.clear();
        vehicles.resize(num_vehicles);

        for (size_t i = 0; i < vehicles.size(); ++i)
        {
            QByteArray vehicle_data;
            stream >> vehicle_data;

            vehicles[i].deserialize(vehicle_data);
        }
    }
};

#endif // SIMULATOR_UPDATE_STRUCT_H
