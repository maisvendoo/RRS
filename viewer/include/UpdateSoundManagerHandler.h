#ifndef UPDATE_SOUND_MANAGER_HANDLER_H
#define UPDATE_SOUND_MANAGER_HANDLER_H

#include "sound-manager.h"
#include <vsg/app/Camera.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class UpdateSoundManagerHandler : public vsg::Inherit<vsg::Visitor, UpdateSoundManagerHandler>
{
public:
    explicit UpdateSoundManagerHandler(vsg::ref_ptr<vsg::Camera> camera, SoundManager *sm);

    void apply(vsg::FrameEvent& frame) override;

private:

    SoundManager *_sound_manager = nullptr;
    vsg::ref_ptr<vsg::LookAt> _lookAt = nullptr;
    vsg::dvec3 _prev_camera_pos = {0.0, 0.0, 0.0};
    double _previousTime = 0.0;
};

#endif // UPDATE_SOUND_MANAGER_HANDLER_H
