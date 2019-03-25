#ifndef     TRAIN_POSITION_H
#define     TRAIN_POSITION_H

#include    <osg/Vec3>
#include    <QMetaType>

struct train_position_t
{
    osg::Vec3   position;
    osg::Vec3   attitude;
    osg::Vec3   driver_pos;

    train_position_t()
        : position(osg::Vec3())
        , attitude(osg::Vec3())
        , driver_pos(osg::Vec3())
    {

    }
};

Q_DECLARE_METATYPE(train_position_t)

#endif // TRAINPOSITION_H
