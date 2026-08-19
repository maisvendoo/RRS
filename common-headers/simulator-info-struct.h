#ifndef     SIMULATOR_INFO_STRUCT_H
#define     SIMULATOR_INFO_STRUCT_H

#include    <QByteArray>
#include    <QDataStream>
#include    <QIODevice>
#include    <QString>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_route_info_t final
{
    double latitude = 47.2;
    double longitude = 39.7;
    QString route_dir_name;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << latitude;
        stream << longitude;
        stream << route_dir_name;

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> latitude;
        stream >> longitude;
        stream >> route_dir_name;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_vehicle_info_t
{
    double vehicle_length = 10.0;
    QString vehicle_config_dir;
    QString vehicle_config_file;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << vehicle_length;
        stream << vehicle_config_dir;
        stream << vehicle_config_file;

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> vehicle_length;
        stream >> vehicle_config_dir;
        stream >> vehicle_config_file;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_vehicles_info_t final
{
    std::vector<simulator_vehicle_info_t> vehicles;

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

    void deserialize(QByteArray &data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        uint32_t num;
        stream >> num;

        vehicles.clear();
        vehicles.resize(num);

        for (std::uint32_t i = 0; i < vehicles.size(); ++i)
        {
            QByteArray vehicle_data;
            stream >> vehicle_data;

            vehicles[i].deserialize(vehicle_data);
        }
    }
};

#endif // SIMULATOR_INFO_STRUCT_H
