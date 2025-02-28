#ifndef SOUND_MANAGER_UPDATE_HANDLER_H
#define SOUND_MANAGER_UPDATE_HANDLER_H

#include "sound-manager.h"
#include <vsg/app/Camera.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SoundManagerUpdateHandler : public vsg::Inherit<vsg::Visitor, SoundManagerUpdateHandler>
{
public:
    explicit SoundManagerUpdateHandler(vsg::ref_ptr<vsg::Camera> camera, SoundManager *sm);

    void apply(vsg::FrameEvent& frame) override;

private:

    SoundManager *_sound_manager;
    vsg::ref_ptr<vsg::LookAt> _lookAt = nullptr;
    vsg::dvec3 _prev_camera_pos = {0.0, 0.0, 0.0};
    double _previousTime;
};

#endif // SOUND_MANAGER_UPDATE_HANDLER_H
