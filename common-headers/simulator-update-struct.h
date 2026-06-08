#ifndef SIMULATOR_UPDATE_STRUCT_H
#define SIMULATOR_UPDATE_STRUCT_H

#include <datetime.h>

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QString>

#include <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_update_players_t final
{
    std::vector<int> clients_id;
    std::vector<int> current_vehicles;
    std::vector<int> controlled_vehicles;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << static_cast<std::uint32_t>(clients_id.size());
        for (auto id : clients_id)
        {
            stream << id;
        }

        stream << static_cast<std::uint32_t>(current_vehicles.size());
        for (auto veh : current_vehicles)
        {
            stream << veh;
        }

        stream << static_cast<std::uint32_t>(controlled_vehicles.size());
        for (auto veh : controlled_vehicles)
        {
            stream << veh;
        }

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        std::uint32_t num;

        stream >> num;
        clients_id.clear();
        clients_id.resize(num);
        for (auto& client_id : clients_id)
        {
            stream >> client_id;
        }

        stream >> num;
        current_vehicles.clear();
        current_vehicles.resize(num);
        for (auto& current_vehicle : current_vehicles)
        {
            stream >> current_vehicle;
        }

        stream >> num;
        controlled_vehicles.clear();
        controlled_vehicles.resize(num);
        for (auto& controlled_vehicle : controlled_vehicles)
        {
            stream >> controlled_vehicle;
        }
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_vehicle_pos_update_t final
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

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

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

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

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
struct simulator_update_pos_t final
{
    int speed_factor = 1;
    simulator_time_t sim_time;
    std::vector<simulator_vehicle_pos_update_t> vehicles;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << speed_factor;
        stream << sim_time.serialize();

        stream << static_cast<std::uint32_t>(vehicles.size());
        for (const auto& vehicle_pos : vehicles)
        {
            stream << vehicle_pos.serialize();
        }

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> speed_factor;

        QByteArray sim_time_data;
        stream >> sim_time_data;
        sim_time.deserialize(sim_time_data);

        std::uint32_t num;

        stream >> num;
        vehicles.clear();
        vehicles.resize(num);

        for (auto& vehicle : vehicles)
        {
            QByteArray vehicle_data;
            stream >> vehicle_data;

            vehicle.deserialize(vehicle_data);
        }
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_vehicle_update_t final
{
    int orientation = 1;
    int train_id = 0;
    int prev_vehicle = -1;
    int next_vehicle = -1;
    std::vector<float> analogSignal;
    /// Сериализованные данные графика движения
    QByteArray timetableData;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << orientation;
        stream << train_id;
        stream << prev_vehicle;
        stream << next_vehicle;

        stream << static_cast<std::uint32_t>(analogSignal.size());

        for (auto signal : analogSignal)
        {
            stream << signal;
        }

        quint32 tt_data_size = timetableData.size();
        stream << tt_data_size;

        if (tt_data_size != 0)
        {
            stream << timetableData;
        }

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> orientation;
        stream >> train_id;
        stream >> prev_vehicle;
        stream >> next_vehicle;

        std::uint32_t num;
        stream >> num;
        analogSignal.clear();
        analogSignal.resize(num);

        for (auto& signal : analogSignal)
        {
            stream >> signal;
        }

        quint32 tt_data_size = 0;
        stream >> tt_data_size;

        if (tt_data_size != 0)
        {
            stream >> timetableData;
        }
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_vehicles_update_t final
{
    std::vector<simulator_vehicle_update_t> vehicles;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << static_cast<std::uint32_t>(vehicles.size());

        for (const auto& vehicle : vehicles)
        {
            stream << vehicle.serialize();
        }

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        std::uint32_t num;
        stream >> num;

        vehicles.clear();
        vehicles.resize(num);

        for (auto& vehicle : vehicles)
        {
            QByteArray vehicle_data;
            stream >> vehicle_data;

            vehicle.deserialize(vehicle_data);
        }
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_train_update_t final
{
    int  first_vehicle_id = 0;
    int  last_vehicle_id = 0;
    QString train_name = "";

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << first_vehicle_id;
        stream << last_vehicle_id;
        stream << train_name;

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> first_vehicle_id;
        stream >> last_vehicle_id;
        stream >> train_name;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_trains_update_t final
{
    std::vector<simulator_train_update_t> trains;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << static_cast<std::uint32_t>(trains.size());

        for (const auto& train : trains)
        {
            stream << train.serialize();
        }

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        std::uint32_t num;
        stream >> num;

        trains.clear();
        trains.resize(num);

        for (auto& train : trains)
        {
            QByteArray train_data;
            stream >> train_data;

            train.deserialize(train_data);
        }
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_vehicle_controlled_update_t final
{
    int current_vehicle = 0;
    QString currentDebugMsg;
    int controlled_vehicle = 0;
    QString controlledDebugMsg;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << current_vehicle;
        stream << currentDebugMsg;
        stream << controlled_vehicle;
        stream << controlledDebugMsg;

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> current_vehicle;
        stream >> currentDebugMsg;
        stream >> controlled_vehicle;
        stream >> controlledDebugMsg;
    }
};

#endif // SIMULATOR_UPDATE_STRUCT_H
