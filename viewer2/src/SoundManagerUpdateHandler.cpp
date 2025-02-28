#include "SoundManagerUpdateHandler.h"
#include "vsg/ui/ApplicationEvent.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SoundManagerUpdateHandler::SoundManagerUpdateHandler(vsg::ref_ptr<vsg::Camera> camera, SoundManager *sm)
    : _lookAt(camera->viewMatrix.cast<vsg::LookAt>())
    , _sound_manager(sm)
{
    if (!_lookAt)
    {
        _lookAt = new vsg::LookAt;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManagerUpdateHandler::apply(vsg::FrameEvent& frame)
{
    double t = frame.frameStamp->simulationTime;
    double dt = t - _previousTime;

    if (dt < 1e-5)
        return;

    vsg::vec3 pos = vsg::vec3(_lookAt->eye);
    vsg::vec3 velocity = vsg::vec3((_lookAt->eye - _prev_camera_pos) / dt);
    vsg::vec3 front = vsg::normalize(vsg::vec3(_lookAt->center - _lookAt->eye));
    vsg::vec3 up = vsg::vec3(_lookAt->up);

    _sound_manager->setListenerPosition(pos.x, pos.y, pos.z);
    _sound_manager->setListenerVelocity(velocity.x, velocity.y, velocity.z);
    _sound_manager->setListenerOrientation(front.x, front.y, front.z, up.x, up.y, up.z);

    _previousTime = t;
    _prev_camera_pos = _lookAt->eye;
}
