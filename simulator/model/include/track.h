#ifndef     TRACK_H
#define     TRACK_H

#include    "vector.hpp"
#include    "math.hpp"

enum
{
    DIMENTION = 3
};

using namespace vmml;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct track_t
{
    /// Длина трека
    double length;

    double x0;

    double x1;

    /// Начальная точка трека
    vec3d   p0;
    /// Конечная точка трека
    vec3d   p1;

    /// Орт трека
    vec3d   orth;
    /// Траверс трека
    vec3d   trav;
    /// Вектор, напраяленный перпендикулярно треку вверх
    vec3d   up;

    track_t()
        : length(0.0)
        , x0(0)
        , x1(0)
        , p0(vec3d())
        , p1(vec3d())
        , orth(vec3d())
        , trav(vec3d())
        , up(vec3d())
    {

    }

    track_t(vec3d p0, vec3d p1)
    {
        this->p0 = p0;
        this->p1 = p1;

        Vector<DIMENTION, double> t = p1 - p0;

        length = t.length();

        orth = (1 / length) * t;

        trav.z() = 0.0;
        trav.x() = t.y();
        trav.y() = t.x();

        trav = trav * (1 / trav.length());

        up = trav.cross(t);
        up = up * (1 / up.length());
    }
};

#endif // TRACK_H
