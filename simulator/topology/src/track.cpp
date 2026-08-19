#include "track.h"

#include "vec3.h"

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

track_t::track_t() = default;

track_t::track_t(const dvec3& p0, const dvec3& p1)
{
    begin_point = p0;
    end_point = p1;
    calc_parameters();
}

QByteArray track_t::serialize() const
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

void track_t::deserialize(QByteArray& data)
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

void track_t::calc_parameters()
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
