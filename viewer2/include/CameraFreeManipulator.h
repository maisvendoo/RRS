#ifndef CAMERA_FREE_MANIPULATOR_H
#define CAMERA_FREE_MANIPULATOR_H

#include "settings.h"
#include <vsg/app/Camera.h>
#include <vsg/maths/transform.h>
#include <vsg/ui/Keyboard.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/ScrollWheelEvent.h>
#include <vsg/ui/TouchEvent.h>

/// Trackball is an event handler that provides mouse and touch controlled 3d trackball camera view manipulation.
class VSG_DECLSPEC CameraFreeManipulator : public vsg::Inherit<vsg::Visitor, CameraFreeManipulator>
{
public:
    explicit CameraFreeManipulator(vsg::ref_ptr<vsg::Camera> camera, settings_t &settings);

    /// compute non dimensional window coordinate (-1,1) from event coords
    vsg::dvec2 ndc(const vsg::PointerEvent& event);

    /// compute trackball coordinate from event coords
    vsg::dvec3 tbc(const vsg::PointerEvent& event);

    void apply(vsg::KeyPressEvent& keyPress) override;
    void apply(vsg::KeyReleaseEvent& keyRelease) override;
    void apply(vsg::FocusInEvent& focusIn) override;
    void apply(vsg::FocusOutEvent& focusOut) override;
    void apply(vsg::ButtonPressEvent& buttonPress) override;
    void apply(vsg::ButtonReleaseEvent& buttonRelease) override;
    void apply(vsg::MoveEvent& moveEvent) override;
    void apply(vsg::ScrollWheelEvent& scrollWheel) override;
    void apply(vsg::TouchDownEvent& touchDown) override;
    void apply(vsg::TouchUpEvent& touchUp) override;
    void apply(vsg::TouchMoveEvent& touchMove) override;
    void apply(vsg::FrameEvent& frame) override;

    void rotate_around(double angle, const vsg::dvec3& axis);
    void rotate_view(const vsg::dvec2& delta);
    void zoom(double coeff);
    void move(const vsg::dvec3& delta);

    std::pair<int32_t, int32_t> cameraRenderAreaCoordinates(const vsg::PointerEvent& pointerEvent) const;
    bool withinRenderArea(const vsg::PointerEvent& pointerEvent) const;

    /// set the LookAt viewport to the specified lookAt, animating the movements from the current lookAt to the new one.
    /// A value of 0.0 instantly moves the lookAt to the new value.
    void setViewpoint(vsg::ref_ptr<vsg::LookAt> lookAt,
                      vsg::ref_ptr<vsg::Perspective> perspective,
                      double duration = 1.0);

    /// Key that turns the view left around the eye points
    vsg::KeySymbol turnLeftKey = vsg::KEY_a;

    /// Key that turns the view right around the eye points
    vsg::KeySymbol turnRightKey = vsg::KEY_d;

    /// Key that pitches up the view around the eye point
    vsg::KeySymbol pitchUpKey = vsg::KEY_w;

    /// Key that pitches down the view around the eye point
    vsg::KeySymbol pitchDownKey = vsg::KEY_s;

    /// Key that moves the view forward
    vsg::KeySymbol moveForwardKey = vsg::KEY_Up;

    /// Key that moves the view backwards
    vsg::KeySymbol moveBackwardKey = vsg::KEY_Down;

    /// Key that moves the view left
    vsg::KeySymbol moveLeftKey = vsg::KEY_Left;

    /// Key that moves the view right
    vsg::KeySymbol moveRightKey = vsg::KEY_Right;
/*
    /// Key that moves the view upward
    vsg::KeySymbol moveUpKey = vsg::KEY_Up;

    /// Key that moves the view downward
    vsg::KeySymbol moveDownKey = vsg::KEY_Down;
*/
    /// Button mask value used to enable rotating the view, defaults to right mouse button
    vsg::ButtonMask rotateButtonMask = vsg::BUTTON_MASK_3;

    /// Button mask value used to moving camera horizontally, defaults to middle mouse button
    vsg::ButtonMask moveButtonMask = vsg::BUTTON_MASK_2;

    /// Button mask value used used for touch events
    vsg::ButtonMask touchMappedToButtonMask = vsg::BUTTON_MASK_1;

protected:

    settings_t& _settings;

    vsg::ref_ptr<vsg::Keyboard> _keyboard;

    vsg::ref_ptr<vsg::Camera> _camera;
    vsg::ref_ptr<vsg::LookAt> _lookAt;
    vsg::ref_ptr<vsg::Perspective> _perspective;

    bool _hasKeyboardFocus = false;
    bool _hasPointerFocus = false;
    bool _lastPointerEventWithinRenderArea = false;

    vsg::time_point _previousTime;
    vsg::ref_ptr<vsg::PointerEvent> _previousPointerEvent;
    double _prevZoomTouchDistance = 0.0;

    bool _prevCtrl = false;
    bool _prevShift = false;
    double _cameraMoveCoeff = 1.0;
    double _pitch_min = vsg::radians(-70.0);
    double _pitch_max = vsg::radians(70.0);

    vsg::time_point _startTime;
    vsg::ref_ptr<vsg::LookAt> _startLookAt;
    vsg::ref_ptr<vsg::LookAt> _endLookAt;
    vsg::ref_ptr<vsg::Perspective> _startPerspective;
    vsg::ref_ptr<vsg::Perspective> _endPerspective;
    std::map<uint32_t, vsg::ref_ptr<vsg::TouchEvent>> _previousTouches;

    double _animationDuration = 0.0;
};

#endif // CAMERA_FREE_MANIPULATOR_H
