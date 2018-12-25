//------------------------------------------------------------------------------
//
//      Motion blur effect operation (view post processing)
//      (c) maisvendoo, 02/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Motion blur effect operation (view post processing)
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 02/12/2018
 */

#ifndef     MOTION_BLUR_H
#define     MOTION_BLUR_H

#include    <osg/Object>
#include    <osgUtil/Optimizer>

/*!
 * \class
 * \brief Motion blur operation
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MotionBlurOperation : public osg::Operation
{
public:

    /// Constructor
    MotionBlurOperation(double persistence)
        : osg::Referenced(true)
        , osg::Operation("MotionBlur", true)
        , _persistence(persistence)
        , _t0(0.0)
        , _cleared(false)
    {

    }

    /// Motion blur handler
    virtual void operator() (osg::Object *object);

protected:

    double _persistence;
    double _t0;
    bool _cleared;
};

#endif // MOTION_BLUR_H
