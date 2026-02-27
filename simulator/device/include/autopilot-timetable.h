#ifndef     AUTOPILOT_TIMETABLE_H
#define     AUTOPILOT_TIMETABLE_H

#include    <QString>
#include    <QBuffer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct autopilot_station_t
{
    /// Название станции
    QString name = "";
    /// Время прибытия
    QString arr_time = "";
    /// То же время, но в секундах от старта игры
    double arr_time_sec = 0.0;
    /// Время отправления
    QString dep_time = "";
    /// То же время, но в секундах от старта игры
    double dep_time_sec = 0.0;
    /// Целевая траектория
    QString target_traj = "";
    /// Координата вдоль целевой траектории
    double coord = 0.0;

    QByteArray serialize()
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << name;
        stream << arr_time;
        stream << arr_time_sec;
        stream << dep_time;
        stream << dep_time_sec;
        stream << target_traj;
        stream << coord;

        return data;
    }

    void deserialize(QByteArray &data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> name;
        stream >> arr_time;
        stream >> arr_time_sec;
        stream >> dep_time;
        stream >> dep_time_sec;
        stream >> target_traj;
        stream >> coord;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct autopilot_timetable_t
{
    QString train_name = "";
    int train_idx = 0;
    std::vector<autopilot_station_t> stations;

    QByteArray serialize()
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << train_name;
        stream << train_idx;
        stream << static_cast<quint32>(stations.size());

        for (auto station : stations)
        {
            stream << station.serialize();
        }

        return data;
    }

    void deserialize(QByteArray &data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> train_name;
        stream >> train_idx;

        quint32 size = 0;
        stream >> size;

        stations.clear();

        for (quint32 i = 0; i < size; ++i)
        {
            autopilot_station_t station;
            QByteArray st_data;
            stream >> st_data;

            station.deserialize(st_data);

            stations.push_back(station);
        }
    }
};


#endif
