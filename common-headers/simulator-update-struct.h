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
    double  position_x = 0.0;
    double  position_y = 0.0;
    double  position_z = 0.0;
    double  orth_x = 0.0;
    double  orth_y = 0.0;
    double  orth_z = 0.0;
    double  up_x = 0.0;
    double  up_y = 0.0;
    double  up_z = 1.0;
    int     train_id = 0;
    int     orientation = 1;
    int     prev_vehicle = -1;
    int     next_vehicle = -1;
    double  length = 10.0;
    std::array<float, MAX_ANALOG_SIGNALS>   analogSignal;

    simulator_vehicle_update_t()
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
        stream << train_id;
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
        stream >> train_id;
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
struct simulator_train_update_t
{
    int  first_vehicle_id = 0;
    int  last_vehicle_id = 0;
    //QString train_ID = "";

    QByteArray serialize()
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << first_vehicle_id;
        stream << last_vehicle_id;
        //stream << train_ID;
        return buff.data();
    }

    void deserialize(QByteArray &data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> first_vehicle_id;
        stream >> last_vehicle_id;
        //stream >> train_ID;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_update_t
{
    double  time = 0.0;
    int     current_vehicle = 0;
    wchar_t currentDebugMsg[DEBUG_STRING_SIZE] = L"";
    int     controlled_vehicle = 0;
    wchar_t controlledDebugMsg[DEBUG_STRING_SIZE] = L"";
    int     num_trains = 0;
    std::array<simulator_train_update_t, MAX_NUM_TRAINS>  trains;
    std::array<simulator_vehicle_update_t, MAX_NUM_VEHICLES>  vehicles;

    simulator_update_t()
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
    std::vector<simulator_train_update_t> trains;
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

        stream << (quint32)trains.size();

        for (auto train : trains)
        {
            stream << train.serialize();
        }

        stream << (quint32)vehicles.size();

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

        quint32 num;
        stream >> num;

        trains.clear();
        trains.resize(num);

        for (size_t i = 0; i < trains.size(); ++i)
        {
            QByteArray train_data;
            stream >> train_data;

            trains[i].deserialize(train_data);
        }

        stream >> num;

        vehicles.clear();
        vehicles.resize(num);

        for (size_t i = 0; i < vehicles.size(); ++i)
        {
            QByteArray vehicle_data;
            stream >> vehicle_data;

            vehicles[i].deserialize(vehicle_data);
        }
    }
};

#endif // SIMULATOR_UPDATE_STRUCT_H
