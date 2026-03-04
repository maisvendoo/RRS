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
    QString arr_time = "-";
    /// Фактическое время прибытия
    QString fact_arr_time = "-";
    /// То же время, но в секундах от старта игры
    double arr_time_sec = 0.0;
    /// Фактическое время прибытия в секундах
    double fact_arr_time_sec = 0.0;
    /// Признак прибытия
    bool is_arrival = false;
    /// Время отправления
    QString dep_time = "-";
    /// Фактическое время отправления
    QString fact_dep_time = "-";
    /// То же время, но в секундах от старта игры
    double dep_time_sec = 0.0;
    /// Фактическое время отправления в секундах
    double fact_dep_time_sec = 0.0;
    /// Признак отправления
    bool is_departure = false;
    /// Целевая траектория
    QString target_traj = "";
    /// Координата вдоль целевой траектории
    double coord = 0.0;
    /// Признак опоздания
    bool is_delay = false;

    QByteArray serialize()
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << name;
        stream << arr_time;
        stream << fact_arr_time;
        stream << arr_time_sec;
        stream << fact_arr_time_sec;
        stream << is_arrival;
        stream << dep_time;
        stream << fact_dep_time;
        stream << dep_time_sec;
        stream << fact_dep_time_sec;
        stream << is_departure;
        stream << target_traj;
        stream << coord;
        stream << is_delay;

        return data;
    }

    void deserialize(QByteArray &data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> name;
        stream >> arr_time;
        stream >> fact_arr_time;
        stream >> arr_time_sec;
        stream >> fact_arr_time_sec;
        stream >> is_arrival;
        stream >> dep_time;
        stream >> fact_dep_time;
        stream >> dep_time_sec;
        stream >> fact_dep_time_sec;
        stream >> is_departure;
        stream >> target_traj;
        stream >> coord;
        stream >> is_delay;
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
