#ifndef     SIMULATOR_UPDATE_STRUCT_H
#define     SIMULATOR_UPDATE_STRUCT_H

#include    <QString>
#include    <QByteArray>
#include    <QBuffer>
#include    <QDataStream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_vehicle_pos_update_t
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

    QByteArray serialize()
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << position_x;
        stream << position_y;
        stream << position_z;
        float tmp;
        tmp = static_cast<float>(orth_x);
        stream << tmp;
        tmp = static_cast<float>(orth_y);
        stream << tmp;
        tmp = static_cast<float>(orth_z);
        stream << tmp;
        tmp = static_cast<float>(up_x);
        stream << tmp;
        tmp = static_cast<float>(up_y);
        stream << tmp;
        tmp = static_cast<float>(up_z);
        stream << tmp;

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
        float tmp;
        stream >> tmp;
        orth_x = static_cast<double>(tmp);
        stream >> tmp;
        orth_y = static_cast<double>(tmp);
        stream >> tmp;
        orth_z = static_cast<double>(tmp);
        stream >> tmp;
        up_x = static_cast<double>(tmp);
        stream >> tmp;
        up_y = static_cast<double>(tmp);
        stream >> tmp;
        up_z = static_cast<double>(tmp);
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_update_pos_t
{
    double time = 0.0;
    int current_vehicle = 0;
    int controlled_vehicle = 0;
    std::vector<simulator_vehicle_pos_update_t> vehicles;

    simulator_update_pos_t()
    {

    }

    QByteArray serialize()
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << time;

        stream << vehicles.size();

        for (auto veh_pos : vehicles)
        {
            stream << veh_pos.serialize();
        }

        return buff.data();
    }

    void deserialize(QByteArray &data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> time;

        size_t num;
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_vehicle_update_t
{
    int orientation = 1;
    int train_id = 0;
    int prev_vehicle = -1;
    int next_vehicle = -1;
    std::vector<float>   analogSignal;

    simulator_vehicle_update_t()
    {

    }

    QByteArray serialize()
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << orientation;
        stream << train_id;
        stream << prev_vehicle;
        stream << next_vehicle;

        stream << analogSignal.size();

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

        stream >> orientation;
        stream >> train_id;
        stream >> prev_vehicle;
        stream >> next_vehicle;

        size_t num;
        stream >> num;
        analogSignal.clear();
        analogSignal.resize(num);

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
    std::vector<simulator_train_update_t> trains;
    std::vector<simulator_vehicle_update_t> vehicles;

    simulator_update_t()
    {

    }

    QByteArray serialize()
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << trains.size();

        for (auto train : trains)
        {
            stream << train.serialize();
        }

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

        size_t num;
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_vehicle_controlled_update_t
{
    int current_vehicle = 0;
    QString currentDebugMsg = "";
    int controlled_vehicle = 0;
    QString controlledDebugMsg = "";

    simulator_vehicle_controlled_update_t()
    {

    }

    QByteArray serialize()
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << current_vehicle;
        stream << currentDebugMsg;
        stream << controlled_vehicle;
        stream << controlledDebugMsg;

        return buff.data();
    }

    void deserialize(QByteArray &data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> current_vehicle;
        stream >> currentDebugMsg;
        stream >> controlled_vehicle;
        stream >> controlledDebugMsg;
    }
};

#endif // SIMULATOR_UPDATE_STRUCT_H
