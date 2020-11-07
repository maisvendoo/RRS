#include    "animation-manager.h"

#include    <osgViewer/Viewer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AnimationManager::AnimationManager(animations_t *animations)
    : osgGA::GUIEventHandler ()
    , animations(animations)
    , start_time(0.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AnimationManager::handle(const osgGA::GUIEventAdapter &ea,
                              osgGA::GUIActionAdapter &aa)
{
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::FRAME:
        {
            osgViewer::Viewer *viewer = dynamic_cast<osgViewer::Viewer *>(&aa);

            if (!viewer)
                break;

            double time = viewer->getFrameStamp()->getReferenceTime();
            double delta_time = time - start_time;
            start_time = time;

            if (delta_time <= 0.04)
            {
                step(static_cast<float>(time), static_cast<float>(delta_time));
            }

            break;
        }

    default:

        break;
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AnimationManager::step(float t, float dt)
{
    for (auto it = animations->begin(); it != animations->end(); ++it)
    {
        it.value()->step(t, dt);
    }
}
