#ifndef     ANIMATION_MANAGER_H
#define     ANIMATION_MANAGER_H

#include    <QMap>
#include    <osgGA/GUIEventHandler>

#include    "animations-list.h"
#include    "proc-animation.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AnimationManager : public osgGA::GUIEventHandler
{
public:

    AnimationManager(animations_t  *animations);

    bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

private:

    animations_t    *animations;
    double          start_time;

    void step(float t, float dt);
};

#endif // ANIMATION_MANAGER_H
