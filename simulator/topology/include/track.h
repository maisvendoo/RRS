#ifndef     TRACK_H
#define     TRACK_H

#include    <vec3.h>

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

    /// Уклон профиля пути, в тясячных
    double inclination = 0.0;

    /// Текущая кривизна траектории в плане
    double curvature = 0.0;

    /// Координата на траектории, соответствующая началу данного трека
    double traj_coord = 0.0;

    track_t()
    {

    }

    track_t(dvec3 p0, dvec3 p1)
    {
        begin_point = p0;
        end_point = p1;

        dvec3 t = end_point - begin_point;
        len = length(t);

        orth = normalize(t);

        inclination = orth.z * 1000.0;

        trav.z = 0.0;
        trav.x = t.y;
        trav.y = t.x;

        trav = normalize(trav);

        up = cross(trav, t);
        up = normalize(up);
    }
};

#endif
