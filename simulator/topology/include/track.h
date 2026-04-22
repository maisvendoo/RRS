#ifndef     TRACK_H
#define     TRACK_H

#include    "vec3.h"

#include    <QByteArray>
#include    <QDataStream>
#include    <QIODevice>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct track_t
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

    track_t() = default;

    track_t(const dvec3& p0, const dvec3& p1)
    {
        begin_point = p0;
        end_point = p1;
        calc_parameters();
    }

    /// Сериализация (прeобразование в последовательность байт)
    QByteArray serialize()
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << begin_point.x << begin_point.y << begin_point.z
               << end_point.x << end_point.y << end_point.z
               << curvature
               << traj_coord
               << railway_coord0
               << railway_coord1;

        return data;
    }

    /// Десериализация
    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> begin_point.x >> begin_point.y >> begin_point.z
               >> end_point.x >> end_point.y >> end_point.z
               >> curvature
               >> traj_coord
               >> railway_coord0
               >> railway_coord1;

        calc_parameters();
    }

private:
    void calc_parameters()
    {
        dvec3 t = end_point - begin_point;
        len = length(t);

        orth = normalize(t);

        inclination = orth.z * 1000.0;

        // trav = cross(orth, {0.0, 0.0, 1.0});
        trav.x = t.y;
        trav.y = -t.x;
        trav.z = 0.0;

        trav = normalize(trav);

        up = cross(trav, t);
        up = normalize(up);
    }
};

#endif // TRACK_H
