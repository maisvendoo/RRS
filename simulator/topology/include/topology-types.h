#ifndef     TOPOLOGY_TYPES_H
#define     TOPOLOGY_TYPES_H

#include    "topology-export.h"
#include    "signal-types.h"

#include    <QByteArray>
#include    <QString>
#include    <QVector>

#include    <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TOPOLOGY_EXPORT topology_pos_t
{
    QString traj_name = "";
    double  traj_coord = 0.0;
    int     dir = 1;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TOPOLOGY_EXPORT topology_station_t
{
    QString name = "";
    double  pos_x = 0.0;
    double  pos_y = 0.0;
    double  pos_z = 0.0;

    QByteArray serialize() const;
    void deserialize(QByteArray& data);
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using topology_stations_list_t = QVector<topology_station_t>;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TOPOLOGY_EXPORT traj_busy_state_t
{
    QString name = "";
    bool    is_busy = false;
    bool    in_route = false;

    QByteArray serialize() const;
    void deserialize(QByteArray& data);
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TOPOLOGY_EXPORT profile_segment_t
{
    /// Дистанция от точки отсчёта профиля, м (вперёд по ходу - «+», назад - «-»)
    double distance = 0.0;

    /// Высота пути, м
    double elevation = 0.0;

    /// Железнодорожный пикетаж, м
    double railway_coord = 0.0;

    /// Уклон сегмента, в тысячных
    double inclination = 0.0;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TOPOLOGY_EXPORT profile_vehicle_t
{
    /// Модель-индекс ПЕ
    size_t vehicle_id = 0;

    /// Начало занимаемого интервала по дистанции профиля, м
    double begin_distance = 0.0;

    /// Конец занимаемого интервала по дистанции профиля, м
    double end_distance = 0.0;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TOPOLOGY_EXPORT profile_signal_t
{
    /// Дистанция светофора от точки отсчёта профиля, м (вперёд по ходу - «+», назад - «-»)
    double distance = 0.0;

    /// Тип светофора (суффикс модели: line/entr/rout/exit/shnt)
    QString signal_type = "";

    /// Состояние линз светофора
    lens_state_t lens = {};

    /// Литер светофора
    QString letter = "";
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TOPOLOGY_EXPORT profile_segments_t
{
    /// Фактические протяжённости профиля назад и вперёд, м
    double backward = 0.0;
    double forward = 0.0;

    /// Вершины ломаной профиля, упорядочены по distance
    std::vector<profile_segment_t> points;

    /// Единицы подвижного состава на профиле (включая другие поезда),
    /// упорядочены по begin_distance
    std::vector<profile_vehicle_t> vehicles;

    /// Светофоры на профиле, упорядочены по distance
    std::vector<profile_signal_t> signal_list;
};

#endif // TOPOLOGY_TYPES_H
