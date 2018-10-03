#ifndef     TRACK_H
#define     TRACK_H

#include    <QString>
#include    <QList>

#include    <osg/Geometry>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct track_t
{
    int         ordinate;
    int         voltage;
    QString     arrows;
    osg::Vec3f  begin_point;
    osg::Vec3f  end_point;
    int         prev_uid;
    int         next_uid;

    float       length;
    osg::Vec3f  orth;
    osg::Vec3f  right;
    float       rail_coord;

    track_t()
        : ordinate(0)
        , voltage(0)
        , arrows("")
        , begin_point(osg::Vec3f(0, 0, 0))
        , end_point(osg::Vec3f(0, 0, 0))
        , prev_uid(-1)
        , next_uid(-2)
        , length(0)
        , orth(osg::Vec3f(0, 0, 0))
        , right(osg::Vec3f(0, 0, 0))
        , rail_coord(0)
    {

    }
};

typedef QList<track_t>  tracks_data_t;

#endif // TRACK_H
