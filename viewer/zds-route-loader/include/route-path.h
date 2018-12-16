//------------------------------------------------------------------------------
//
//      Tracks loader for ZDSimulator routes
//      (c) maisvendoo, 26/11/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Tracks loader for ZDSimulator routes
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 26/11/2018
 */

#ifndef     ROUTE_PATH_H
#define     ROUTE_PATH_H

#include    <fstream>

#include    <osg/Referenced>
#include    <osgDB/FileUtils>
#include    <osgDB/FileNameUtils>

#include    "track.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class RoutePath : public osg::Referenced
{
public:

    /// Constructor with automatic track loading
    RoutePath(const std::string &track_file_path);    

    /// Get cartesian position of track point
    osg::Vec3 getPosition(float railway_coord);

    /// Get cartesian position and camera's attitude
    osg::Vec3 getPosition(float railway_coord, osg::Vec3 &attitude);

    /// Get full length of tracks
    float getLength() const;

    /// Get visual track line node (for debug)
    osg::Group *getTrackLine(const osg::Vec4 &color);

protected:

    /// Tracks full length
    float                   length;
    /// Tracks data
    std::vector<track_t>    track_data;

    /// Load tracks from file *.trk
    bool load(const std::string &track_file_path);

    /// Load tracks from input stream
    bool load(std::istream &stream);

    /// Get line from input stream
    std::string getLine(std::istream &stream) const;

    /// Find track for some railway coordinate
    track_t findTrack(float railway_coord, track_t &next_track);
};

#endif // ROUTE_PATH_H
