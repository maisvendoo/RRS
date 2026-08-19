#include "UpdateSoundManagerHandler.h"

#include "sound-manager.h"

#include <vsg/app/ViewMatrix.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsg/ui/ApplicationEvent.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
UpdateSoundManagerHandler::UpdateSoundManagerHandler(vsg::ref_ptr<vsg::LookAt> look_at, SoundManager* sound_manager)
    : sound_manager(sound_manager)
    , look_at(look_at)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateSoundManagerHandler::apply(vsg::FrameEvent& frame)
{
    const double t = frame.frameStamp->simulationTime;
    const double dt = t - prev_time;

    if (dt < 1e-5)
    {
        return;
    }

    const vsg::vec3 pos = vsg::vec3(look_at->eye);
    const vsg::vec3 velocity = vsg::vec3((look_at->eye - prev_camera_pos) / dt);
    const vsg::vec3 front = vsg::normalize(vsg::vec3(look_at->center - look_at->eye));
    const vsg::vec3 up = vsg::vec3(look_at->up);

    sound_manager->setListenerPosition(pos.x, pos.y, pos.z);
    sound_manager->setListenerVelocity(velocity.x, velocity.y, velocity.z);
    sound_manager->setListenerOrientation(front.x, front.y, front.z, up.x, up.y, up.z);

    prev_time = t;
    prev_camera_pos = look_at->eye;
}
