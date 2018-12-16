//------------------------------------------------------------------------------
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     MOTION_BLUR_H
#define     MOTION_BLUR_H

#include    <osg/Object>
#include    <osgUtil/Optimizer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MotionBlurOperation : public osg::Operation
{
public:

    MotionBlurOperation(double persistence)
        : osg::Referenced(true)
        , osg::Operation("MotionBlur", true)
        , _persistence(persistence)
        , _t0(0.0)
        , _cleared(false)
    {

    }

    virtual void operator() (osg::Object *object);

protected:

    double _persistence;
    double _t0;
    bool _cleared;
};

#endif // MOTION_BLUR_H
