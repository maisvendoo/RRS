#ifndef     SIMULATOR_INFO_STRUCT_H
#define     SIMULATOR_INFO_STRUCT_H

#include    <QString>
#include    <QByteArray>
#include    <QBuffer>
#include    <QDataStream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_route_info_t final
{
    QString route_dir_name;

    QByteArray serialize() const
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << route_dir_name;

        return buff.data();
    }

    void deserialize(QByteArray& data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

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
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << vehicle_length;
        stream << vehicle_config_dir;
        stream << vehicle_config_file;

        return buff.data();
    }

    void deserialize(QByteArray& data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

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
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << static_cast<std::uint32_t>(vehicles.size());

        for (const auto& vehicle : vehicles)
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
