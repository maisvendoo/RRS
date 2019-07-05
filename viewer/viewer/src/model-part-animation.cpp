#include    "model-part-animation.h"
#include    "animation-path-callback.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ModelPartAnimation::ModelPartAnimation(osg::MatrixTransform *transform)
    : transform(transform)
    , path(nullptr)
    , pos(0.0)
    , ref_pos(0.0)
    , lastTime(1.0)
{
    osg::AnimationPathCallback *callback = dynamic_cast<osg::AnimationPathCallback *>(transform->getUpdateCallback());

    if (callback == nullptr)
        return;    

    path = callback->getAnimationPath();

    if (!path.valid())
        return;

    path->setLoopMode(osg::AnimationPath::NO_LOOPING);
    lastTime = path->getLastTime();

    transform->removeUpdateCallback(callback);

    update();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelPartAnimation::step(double t, double dt)
{
    (void) t;

    pos += (ref_pos - pos) * dt / lastTime;

    update();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelPartAnimation::setRefPosition(double pos)
{
    ref_pos = pos;
}

void ModelPartAnimation::setLastTime(double lastTime)
{
    this->lastTime = lastTime;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModelPartAnimation::update()
{
    if (path == nullptr)
        return;

    osg::Matrix matrix;
    path->getMatrix(pos * path->getLastTime(), matrix);

    transform->setMatrix(matrix);
}
