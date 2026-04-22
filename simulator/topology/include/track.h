#ifndef     TRACK_H
#define     TRACK_H

#include    "topology-export.h"
#include    "vec3.h"

#include    <QByteArray>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TOPOLOGY_EXPORT track_t
{
    /// Начальная точка трека
    dvec3 begin_point = {0.0, 0.0, 0.0};

    /// Конечная точка трека
    dvec3 end_point = {0.0, 0.0, 0.0};

    /// Единичный вектор вдоль трека из начала в конец
    dvec3 orth = {0.0, 0.0, 0.0};

    /// Траверс трека, вектор, препендикулярный треку в плане
    dvec3 trav = {0.0, 0.0, 0.0};

    /// Вектор, перпендикулярный треку в вертикальной плоскости
    dvec3 up = {0.0, 0.0, 0.0};

    /// Длина трека
    double len = 0.0;

    /// Уклон профиля пути, в тысячных
    double inclination = 0.0;

    /// Текущая кривизна траектории в плане (НЕ ИСПОЛЬЗУЕТСЯ В ДАННЫЙ МОМЕНТ)
    double curvature = 0.0;

    /// Координата на траектории, соответствующая началу данного трека
    double traj_coord = 0.0;

    /// Железнодорожный пикетаж в точке начала данного трека
    double railway_coord0 = 0.0;

    /// Железнодорожный пикетаж в точке конца данного трека
    double railway_coord1 = 0.0;

    track_t();
    track_t(const dvec3& p0, const dvec3& p1);

    QByteArray serialize() const;
    void deserialize(QByteArray& data);

private:
    void calc_parameters();
};

#endif // TRACK_H
