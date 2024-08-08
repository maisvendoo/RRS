#ifndef     ZDS_TRACK_H
#define     ZDS_TRACK_H

#include    <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct point_t
{
    double x = 0;
    double y = 0;
    double z = 0;

    bool operator==(const point_t &p)
    {
        double dx = this->x - p.x;
        double dy = this->y - p.y;
        double dz = this->z - p.z;

        if (dx*dx + dy*dy + dz*dz <= 1e-6)
        {
            return true;
        }

        return false;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct zds_track_t
{
    point_t     begin_point;
    point_t     end_point;
    int         prev_uid = 0;
    int         next_uid = 0;
    std::string arrows = "";
    int         voltage = 0;
    int         ordinate = 0;
};

#endif
