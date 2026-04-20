#ifndef CAMERA_NAVIGATION_STATE_H
#define CAMERA_NAVIGATION_STATE_H

#include "states/State.h"

namespace vsg
{

class KeyPressEvent;

}

class CameraNavigationState : public State
{
public:
    virtual ~CameraNavigationState() override;
    virtual void handle_key_press(vsg::KeyPressEvent& keyPress) override;
};

#endif // CAMERA_NAVIGATION_STATE_H
