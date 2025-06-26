#pragma once
#ifndef UPDATE_SOUND_MANAGER_HANDLER_H
#define UPDATE_SOUND_MANAGER_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/Visitor.h>
#include <vsg/maths/vec3.h>
#include <vsg/ui/ApplicationEvent.h>

class SoundManager;

namespace vsg
{
    class LookAt;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class UpdateSoundManagerHandler final : public vsg::Inherit<vsg::Visitor, UpdateSoundManagerHandler>
{
public:
    UpdateSoundManagerHandler(vsg::ref_ptr<vsg::LookAt> look_at, SoundManager* sound_manager);

    void apply(vsg::FrameEvent& frame) override;

private:
    SoundManager* const sound_manager;
    const vsg::ref_ptr<vsg::LookAt> look_at;
    vsg::dvec3 prev_camera_pos = {0.0, 0.0, 0.0};
    double prev_time = 0.0;
};

#endif // UPDATE_SOUND_MANAGER_HANDLER_H
