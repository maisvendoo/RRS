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

#ifndef TRACK_H
#define TRACK_H

#include <string>
#include <vector>

#include <osg/Vec3>

/*!
 * \struct
 * \brief Route trajectory track
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct track_t
{
    track_t();

    int ordinate;
    int voltage;
    std::string arrows;
    osg::Vec3 begin_point;
    osg::Vec3 end_point;
    int prev_uid;
    int next_uid;

    float length;
    osg::Vec3 orth;
    osg::Vec3 attitude;
    osg::Vec3 right;
    float rail_coord;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using track_data_t = std::vector<track_t>;

#endif // TRACK_H
