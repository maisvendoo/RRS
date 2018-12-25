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

#include    "motion-blur.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MotionBlurOperation::operator()(osg::Object *object)
{
    osg::GraphicsContext *gc = dynamic_cast<osg::GraphicsContext *>(object);

    if (!gc)
        return;

    double t = gc->getState()->getFrameStamp()->getSimulationTime();    

    if (!_cleared)
    {
        glClearColor(0, 0, 0, 0);
        glClear(GL_ACCUM_BUFFER_BIT);
        _cleared = true;
        _t0 = t;
    }

    double dt = fabs(t - _t0);
    _t0 = t;

    float s = static_cast<float>(pow(0.2, dt / _persistence));


    glAccum(GL_MULT, s);
    glAccum(GL_ACCUM, 1.0f - s);
    glAccum(GL_RETURN, 1.0f);
}
