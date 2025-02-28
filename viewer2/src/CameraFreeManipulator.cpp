#include "CameraFreeManipulator.h"

CameraFreeManipulator::CameraFreeManipulator(vsg::ref_ptr<vsg::Camera> camera, settings_t &settings) :
    _settings(settings),
    _camera(camera),
    _lookAt(camera->viewMatrix.cast<vsg::LookAt>()),
    _perspective(camera->projectionMatrix.cast<vsg::Perspective>()),
    _keyboard(vsg::Keyboard::create())
{
    if (!_lookAt)
    {
        _lookAt = new vsg::LookAt;
    }
    if (!_perspective)
    {
        _perspective = new vsg::Perspective;
    }
    _pitch_min = vsg::radians(_settings.pitch_min);
    _pitch_max = vsg::radians(_settings.pitch_max);
}

std::pair<int32_t, int32_t> CameraFreeManipulator::cameraRenderAreaCoordinates(const vsg::PointerEvent& pointerEvent) const
{
    return {pointerEvent.x, pointerEvent.y};
}

bool CameraFreeManipulator::withinRenderArea(const vsg::PointerEvent& pointerEvent) const
{
    auto renderArea = _camera->getRenderArea();
    auto [x, y] = cameraRenderAreaCoordinates(pointerEvent);

    return (x >= renderArea.offset.x && x < static_cast<int32_t>(renderArea.offset.x + renderArea.extent.width)) &&
           (y >= renderArea.offset.y && y < static_cast<int32_t>(renderArea.offset.y + renderArea.extent.height));
}

vsg::dvec2 CameraFreeManipulator::ndc(const vsg::PointerEvent& event)
{
    auto renderArea = _camera->getRenderArea();
    auto [x, y] = cameraRenderAreaCoordinates(event);

    double aspectRatio = static_cast<double>(renderArea.extent.width) / static_cast<double>(renderArea.extent.height);
    vsg::dvec2 v(
        (renderArea.extent.width > 0) ? (static_cast<double>(x - renderArea.offset.x) / static_cast<double>(renderArea.extent.width) * 2.0 - 1.0) * aspectRatio : 0.0,
        (renderArea.extent.height > 0) ? static_cast<double>(y - renderArea.offset.y) / static_cast<double>(renderArea.extent.height) * 2.0 - 1.0 : 0.0);
    return v;
}

vsg::dvec3 CameraFreeManipulator::tbc(const vsg::PointerEvent& event)
{
    vsg::dvec2 v = ndc(event);

    double l = length(v);
    if (l < 1.0f)
    {
        double h = 0.5 + cos(l * vsg::PI) * 0.5;
        return vsg::dvec3(v.x, -v.y, h);
    }
    else
    {
        return vsg::dvec3(v.x, -v.y, 0.0);
    }
}

void CameraFreeManipulator::apply(vsg::KeyPressEvent& keyPress)
{
    if (_keyboard) keyPress.accept(*_keyboard);
}

void CameraFreeManipulator::apply(vsg::KeyReleaseEvent& keyRelease)
{
    if (_keyboard) keyRelease.accept(*_keyboard);
}

void CameraFreeManipulator::apply(vsg::FocusInEvent& focusIn)
{
    if (_keyboard) focusIn.accept(*_keyboard);
}

void CameraFreeManipulator::apply(vsg::FocusOutEvent& focusOut)
{
    if (_keyboard) focusOut.accept(*_keyboard);
}

void CameraFreeManipulator::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        _hasKeyboardFocus = false;
        return;
    }

    _hasPointerFocus = _hasKeyboardFocus = withinRenderArea(buttonPress);
    _lastPointerEventWithinRenderArea = _hasPointerFocus;

    if (_hasPointerFocus) buttonPress.handled = true;

    _previousPointerEvent = &buttonPress;
}

void CameraFreeManipulator::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    if (buttonRelease.handled) return;

    _lastPointerEventWithinRenderArea = withinRenderArea(buttonRelease);
    _hasPointerFocus = false;

    _previousPointerEvent = &buttonRelease;
}

void CameraFreeManipulator::apply(vsg::MoveEvent& moveEvent)
{
    _lastPointerEventWithinRenderArea = withinRenderArea(moveEvent);

    if (moveEvent.handled || !_hasPointerFocus) return;

    vsg::dvec2 new_ndc = ndc(moveEvent);
//    vsg::dvec3 new_tbc = tbc(moveEvent);

    if (!_previousPointerEvent) _previousPointerEvent = &moveEvent;

    vsg::dvec2 prev_ndc = ndc(*_previousPointerEvent);
//    vsg::dvec3 prev_tbc = tbc(*_previousPointerEvent);

    double dt = std::chrono::duration<double, std::chrono::seconds::period>(moveEvent.time - _previousPointerEvent->time).count();

    _previousPointerEvent = &moveEvent;

    if (moveEvent.mask & moveButtonMask)
    {
        moveEvent.handled = true;

        vsg::dvec3 delta((new_ndc - prev_ndc).x, (new_ndc - prev_ndc).y, 0.0);

        if (delta)
            move(delta * dt * _cameraMoveCoeff * _settings.free_cam_speed_mouse);

        return;
    }

    if (moveEvent.mask & rotateButtonMask)
    {
        moveEvent.handled = true;

        vsg::dvec2 delta(prev_ndc - new_ndc);
        if (delta)
            rotate_view(delta * dt * (20.0 + _perspective->fieldOfViewY) * _settings.free_cam_rotate_mouse);
    }
}

void CameraFreeManipulator::apply(vsg::ScrollWheelEvent& scrollWheel)
{
    if (scrollWheel.handled || !_lastPointerEventWithinRenderArea) return;

    scrollWheel.handled = true;

    if (_keyboard->pressed(vsg::KEY_Control_L) || _keyboard->pressed(vsg::KEY_Control_L))
    {
        if (scrollWheel.delta.y > 0.0)
            move(vsg::dvec3(0.0, 0.0, _settings.free_cam_height_step));
        else if (scrollWheel.delta.y < 0.0)
            move(vsg::dvec3(0.0, 0.0, -_settings.free_cam_height_step));
        return;
    }

    if (scrollWheel.delta.y > 0.0)
        zoom(1 / _settings.free_cam_fovy_coeff);
    else if (scrollWheel.delta.y < 0.0)
        zoom(_settings.free_cam_fovy_coeff);
}

void CameraFreeManipulator::apply(vsg::TouchDownEvent& touchDown)
{
    _previousTouches[touchDown.id] = &touchDown;
    switch (touchDown.id)
    {
    case 0: {
        if (_previousTouches.size() == 1)
        {
            vsg::ref_ptr<vsg::Window> w = touchDown.window;
            vsg::ref_ptr<vsg::ButtonPressEvent> evt = vsg::ButtonPressEvent::create(
                w,
                touchDown.time,
                touchDown.x,
                touchDown.y,
                touchMappedToButtonMask,
                touchDown.id);
            apply(*evt.get());
        }
        break;
    }
    case 1: {
        _prevZoomTouchDistance = 0.0;
        if (touchDown.id == 0 && _previousTouches.count(1))
        {
            const auto& prevTouch1 = _previousTouches[1];
            auto a = std::abs(static_cast<double>(prevTouch1->x) - touchDown.x);
            auto b = std::abs(static_cast<double>(prevTouch1->y) - touchDown.y);
            if (a > 0 || b > 0)
                _prevZoomTouchDistance = sqrt(a * a + b * b);
        }
        break;
    }
    }
}

void CameraFreeManipulator::apply(vsg::TouchUpEvent& touchUp)
{
    if (touchUp.id == 0 && _previousTouches.size() == 1)
    {
        vsg::ref_ptr<vsg::Window> w = touchUp.window;
        vsg::ref_ptr<vsg::ButtonReleaseEvent> evt = vsg::ButtonReleaseEvent::create(
            w,
            touchUp.time,
            touchUp.x,
            touchUp.y,
            touchMappedToButtonMask,
            touchUp.id);
        apply(*evt.get());
    }
    _previousTouches.erase(touchUp.id);
}

void CameraFreeManipulator::apply(vsg::TouchMoveEvent& touchMove)
{
    vsg::ref_ptr<vsg::Window> w = touchMove.window;
    switch (_previousTouches.size())
    {
    case 1: {
        // Rotate
        vsg::ref_ptr<vsg::MoveEvent> evt = vsg::MoveEvent::create(
            w,
            touchMove.time,
            touchMove.x,
            touchMove.y,
            touchMappedToButtonMask);
        apply(*evt.get());
        break;
    }
    case 2: {
        if (touchMove.id == 0 && _previousTouches.count(0))
        {
            // Zoom
            const auto& prevTouch1 = _previousTouches[1];
            auto a = std::abs(static_cast<double>(prevTouch1->x) - touchMove.x);
            auto b = std::abs(static_cast<double>(prevTouch1->y) - touchMove.y);
            if (a > 0 || b > 0)
            {
                auto touchZoomDistance = sqrt(a * a + b * b);
                if (_prevZoomTouchDistance && touchZoomDistance > 0)
                {
                    auto zoomLevel = touchZoomDistance / _prevZoomTouchDistance;
                    if (zoomLevel < 1)
                        zoomLevel = -(1 / zoomLevel);
                    zoomLevel *= 0.1;
                    zoom(zoomLevel);
                }
                _prevZoomTouchDistance = touchZoomDistance;
            }
        }
        break;
    }
    }
    _previousTouches[touchMove.id] = &touchMove;
}

void CameraFreeManipulator::apply(vsg::FrameEvent& frame)
{
    // std::cout<<"Trackball::apply(FrameEvent&) frameCount = "<<frame.frameStamp->frameCount<<std::endl;

    double dt = std::chrono::duration<double, std::chrono::seconds::period>(frame.time - _previousTime).count();
    _previousTime = frame.time;

    if (_endLookAt)
    {
        double timeSinceOfAnimation = std::chrono::duration<double, std::chrono::seconds::period>(frame.time - _startTime).count();
        if (timeSinceOfAnimation < _animationDuration)
        {
            double r = vsg::smoothstep(0.0, 1.0, timeSinceOfAnimation / _animationDuration);

            _lookAt->eye = vsg::mix(_startLookAt->eye, _endLookAt->eye, r);
            _lookAt->center = vsg::mix(_startLookAt->center, _endLookAt->center, r);
            _perspective->fieldOfViewY = vsg::mix(_startPerspective->fieldOfViewY, _endPerspective->fieldOfViewY, r);

            double angle = acos(vsg::dot(_startLookAt->up, _endLookAt->up) / (vsg::length(_startLookAt->up) * vsg::length(_endLookAt->up)));
            if (angle > 1.0e-6)
            {
                auto rotation = vsg::rotate(angle * r, vsg::normalize(vsg::cross(_startLookAt->up, _endLookAt->up)));
                _lookAt->up = rotation * _startLookAt->up;
            }
            else
            {
                _lookAt->up = _endLookAt->up;
            }
        }
        else
        {
            _lookAt->eye = _endLookAt->eye;
            _lookAt->center = _endLookAt->center;
            _lookAt->up = _endLookAt->up;
            _perspective->fieldOfViewY = _endPerspective->fieldOfViewY;

            _endLookAt = nullptr;
            _endPerspective = nullptr;
            _animationDuration = 0.0;
        }

        return;
    }

    if (_hasKeyboardFocus && _keyboard)
    {
        auto times2speed = [](std::pair<double, double> duration) -> double {
            if (duration.first <= 0.0) return 0.0;
            double speed = duration.first >= 0.5 ? 1.0 : duration.first * 2.0;

            if (duration.second > 0.0)
            {
                // key has been released so slow down
                speed -= duration.second * 2.0;
                return speed > 0.0 ? speed : 0.0;
            }
            else
            {
                // key still pressed so return speed based on duration of press
                return speed;
            }
        };

        double speed = 0.0;
        vsg::dvec3 move_speed(0.0, 0.0, 0.0);
        if ((speed = times2speed(_keyboard->times(moveRightKey))) != 0.0) move_speed.x += -speed;
        if ((speed = times2speed(_keyboard->times(moveLeftKey))) != 0.0) move_speed.x += speed;
        if ((speed = times2speed(_keyboard->times(moveForwardKey))) != 0.0) move_speed.y += speed;
        if ((speed = times2speed(_keyboard->times(moveBackwardKey))) != 0.0) move_speed.y += -speed;
/*        if ((speed = times2speed(_keyboard->times(moveUpKey))) != 0.0) move_speed.z += speed;
        if ((speed = times2speed(_keyboard->times(moveDownKey))) != 0.0) move_speed.z += -speed;*/

        if (move_speed)
        {
            bool isCtrl = (_keyboard->pressed(vsg::KEY_Control_L) || _keyboard->pressed(vsg::KEY_Control_R));
            if (!_prevCtrl && isCtrl)
                _cameraMoveCoeff = _cameraMoveCoeff / _settings.free_cam_speed_coeff;
            _prevCtrl = isCtrl;

            bool isShift = (_keyboard->pressed(vsg::KEY_Shift_L) || _keyboard->pressed(vsg::KEY_Shift_R));
            if (!_prevShift && isShift)
                _cameraMoveCoeff = _cameraMoveCoeff * _settings.free_cam_speed_coeff;
            _prevShift = isShift;

            move(move_speed * dt * _cameraMoveCoeff * _settings.free_cam_speed_keyboard);
        }
        else
        {
            _cameraMoveCoeff = 1.0;
            _prevCtrl = false;
            _prevShift = false;
        }

        vsg::dvec2 rot_speed(0.0, 0.0);
        if ((speed = times2speed(_keyboard->times(turnRightKey))) != 0.0) rot_speed.x += -speed;
        if ((speed = times2speed(_keyboard->times(turnLeftKey))) != 0.0) rot_speed.x += speed;
        if ((speed = times2speed(_keyboard->times(pitchUpKey))) != 0.0) rot_speed.y += speed;
        if ((speed = times2speed(_keyboard->times(pitchDownKey))) != 0.0) rot_speed.y += -speed;

        if (rot_speed)
        {
            rotate_view(rot_speed * dt * (20.0 + _perspective->fieldOfViewY) * _settings.free_cam_rotate_keyboard);
        }
    }
}

void CameraFreeManipulator::rotate_around(double angle, const vsg::dvec3& axis)
{
    vsg::dmat4 rotation = vsg::rotate(angle, axis);
    vsg::dmat4 lv = vsg::lookAt(_lookAt->eye, _lookAt->center, _lookAt->up);
    vsg::dvec3 centerEyeSpace = (lv * _lookAt->center);

    vsg::dmat4 matrix = vsg::inverse(lv) * vsg::translate(centerEyeSpace) * rotation * vsg::translate(-centerEyeSpace) * lv;

    _lookAt->up = vsg::normalize(matrix * (_lookAt->eye + _lookAt->up) - matrix * _lookAt->eye);
    _lookAt->center = matrix * _lookAt->center;
    _lookAt->eye = matrix * _lookAt->eye;
}

void CameraFreeManipulator::rotate_view(const vsg::dvec2& delta)
{
    vsg::dvec3 lookNormal = vsg::normalize(_lookAt->center - _lookAt->eye);
    vsg::dvec3 forwardNormal = vsg::normalize(vsg::dvec3(lookNormal.x, lookNormal.y, 0.0));
    //vsg::dvec3 upNormal = normalize(_lookAt->up);
    vsg::dvec3 upNormal = vsg::dvec3(0.0, 0.0, 1.0);
    //vsg::dvec3 sideNormal = cross(forwardNormal, _lookAt->up);
    vsg::dvec3 sideNormal = vsg::dvec3(forwardNormal.y, -forwardNormal.x, 0.0);

    double sideAngle = delta.x;
    double upAngle = delta.y;

    double curUpAngle = asin(lookNormal.z);
    if ((curUpAngle + upAngle) > _pitch_max)
        upAngle = _pitch_max - curUpAngle;
    if ((curUpAngle + upAngle) < _pitch_min)
        upAngle = _pitch_min - curUpAngle;

    vsg::dmat4 matrix = vsg::translate(_lookAt->eye) *
                        vsg::rotate(sideAngle, upNormal) *
                        vsg::rotate(upAngle, sideNormal) *
                        vsg::translate(-_lookAt->eye);

    _lookAt->up = vsg::normalize(matrix * (_lookAt->eye + _lookAt->up) - matrix * _lookAt->eye);
    _lookAt->center = matrix * _lookAt->center;
    _lookAt->eye = matrix * _lookAt->eye;
}

void CameraFreeManipulator::zoom(double coeff)
{
    double new_fovy = _perspective->fieldOfViewY;

    new_fovy = new_fovy * coeff;
    if ((new_fovy > _settings.fovy_max) || (new_fovy < _settings.fovy_min))
            return;

    _perspective->fieldOfViewY = new_fovy;
}

void CameraFreeManipulator::move(const vsg::dvec3 &delta)
{
    vsg::dvec3 lookNormal = vsg::normalize(_lookAt->center - _lookAt->eye);
    vsg::dvec3 forwardNormal = vsg::normalize(vsg::dvec3(lookNormal.x, lookNormal.y, 0.0));
    //vsg::dvec3 upNormal = normalize(_lookAt->up);
    vsg::dvec3 upNormal = vsg::dvec3(0.0, 0.0, 1.0);
    //vsg::dvec3 sideNormal = cross(forwardNormal, _lookAt->up);
    vsg::dvec3 sideNormal = vsg::dvec3(forwardNormal.y, -forwardNormal.x, 0.0);

    vsg::dvec3 translation = sideNormal * (-delta.x) +
                             forwardNormal * (delta.y) +
                             upNormal * (delta.z);

    _lookAt->eye = _lookAt->eye + translation;
    _lookAt->center = _lookAt->center + translation;
}

void CameraFreeManipulator::setViewpoint(vsg::ref_ptr<vsg::LookAt> lookAt,
                                         vsg::ref_ptr<vsg::Perspective> perspective,
                                         double duration)
{
    if ((!lookAt) || (!perspective)) return;

    if (duration == 0.0)
    {
        _lookAt->eye = lookAt->eye;
        _lookAt->center = lookAt->center;
        _lookAt->up = lookAt->up;

        _perspective->fieldOfViewY = perspective->fieldOfViewY;

        _startLookAt = nullptr;
        _endLookAt = nullptr;
        _startPerspective = nullptr;
        _endPerspective = nullptr;
        _animationDuration = 0.0;
    }
    else
    {
        _startTime = _previousTime;
        _endLookAt = lookAt;
        _startLookAt = vsg::LookAt::create(*_lookAt);
        _endPerspective = perspective;
        _startPerspective = vsg::Perspective::create(*_perspective);
        _animationDuration = duration;
    }
}
