//------------------------------------------------------------------------------
//
//      Route track's data
//      (c) maisvendoo
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Route track's data
 * \copyright maisvendoo
 * \author maisvendoo
 */

#ifndef     TRACK_H
#define     TRACK_H

#include    <string>
#include    <vector>

#include    <osg/Geometry>

/*!
 * \struct
 * \brief Route trajectory track
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct track_t
{
    int         ordinate;
    int         voltage;
    std::string arrows;
    osg::Vec3   begin_point;
    osg::Vec3   end_point;
    int         prev_uid;
    int         next_uid;

    float       length;
    osg::Vec3   orth;
    osg::Vec3   attitude;
    osg::Vec3   right;
    float       rail_coord;

    track_t()
        : ordinate(0)
        , voltage(0)
        , arrows("")
        , begin_point(osg::Vec3())
        , end_point(osg::Vec3())
        , prev_uid(-1)
        , next_uid(-2)
        , length(0)
        , orth(osg::Vec3())
        , attitude(osg::Vec3())
        , right(osg::Vec3())
        , rail_coord(0.0f)

    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<track_t>    track_data_t;

#endif // TRACK_H
