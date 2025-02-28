#include "CameraCabineManipulator.h"

CameraCabineManipulator::CameraCabineManipulator(vsg::ref_ptr<vsg::Keyboard> keyboard,
                                                 vsg::ref_ptr<vsg::Camera> camera,
                                                 settings_t &settings)
    : CameraAbstract(keyboard, camera, settings)
{
    _pitch_min = vsg::radians(_settings.pitch_min);
    _pitch_max = vsg::radians(_settings.pitch_max);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraCabineManipulator::mouseWheelEvent(vsg::vec3 delta)
{
    if (_keyboard->pressed(vsg::KEY_Control_L) || _keyboard->pressed(vsg::KEY_Control_L))
    {
        if (delta.y > 0.0)
            move(vsg::dvec3(0.0, 0.0, _settings.free_cam_height_step));
        else if (delta.y < 0.0)
            move(vsg::dvec3(0.0, 0.0, -_settings.free_cam_height_step));
        return;
    }

    if (delta.y > 0.0)
        zoom(1 / _settings.free_cam_fovy_coeff);
    else if (delta.y < 0.0)
        zoom(_settings.free_cam_fovy_coeff);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraCabineManipulator::mouseMoveEvent(vsg::ButtonMask button_mask, vsg::dvec2 delta, double dt)
{
    if (button_mask & moveButtonMask)
    {
        vsg::dvec3 move_delta(delta.x, delta.y, 0.0);

        if (move_delta)
            move(move_delta * dt * _cameraMoveCoeff * _settings.free_cam_speed_mouse);

        return;
    }

    if (button_mask & rotateButtonMask)
    {
        if (delta)
            rotate_view(-delta * dt * (20.0 + _perspective->fieldOfViewY) * _settings.free_cam_rotate_mouse);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraCabineManipulator::touchZoomEvent(double zoomLevel)
{
    zoom(zoomLevel);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraCabineManipulator::frameEvent(double dt)
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

    if (_keyboard->pressed(vsg::KEY_Alt_L) || _keyboard->pressed(vsg::KEY_Alt_L))
    {
        vsg::dvec2 rot_speed(0.0, 0.0);
        if ((speed = times2speed(_keyboard->times(turnRightKey))) != 0.0) rot_speed.x += -speed;
        if ((speed = times2speed(_keyboard->times(turnLeftKey))) != 0.0) rot_speed.x += speed;
        if ((speed = times2speed(_keyboard->times(pitchUpKey))) != 0.0) rot_speed.y += speed;
        if ((speed = times2speed(_keyboard->times(pitchDownKey))) != 0.0) rot_speed.y += -speed;

        if (rot_speed)
        {
            rotate_view(rot_speed * dt * (20.0 + _perspective->fieldOfViewY) * _settings.free_cam_rotate_keyboard);
        }
        return;
    }

    vsg::dvec3 move_speed(0.0, 0.0, 0.0);
    if ((speed = times2speed(_keyboard->times(moveRightKey))) != 0.0) move_speed.x += -speed;
    if ((speed = times2speed(_keyboard->times(moveLeftKey))) != 0.0) move_speed.x += speed;
    if ((speed = times2speed(_keyboard->times(moveForwardKey))) != 0.0) move_speed.y += speed;
    if ((speed = times2speed(_keyboard->times(moveBackwardKey))) != 0.0) move_speed.y += -speed;
    //if ((speed = times2speed(_keyboard->times(moveUpKey))) != 0.0) move_speed.z += speed;
    //if ((speed = times2speed(_keyboard->times(moveDownKey))) != 0.0) move_speed.z += -speed;

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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraCabineManipulator::rotate_around(double angle, const vsg::dvec3& axis)
{
    vsg::dmat4 rotation = vsg::rotate(angle, axis);
    vsg::dmat4 lv = vsg::lookAt(_lookAt->eye, _lookAt->center, _lookAt->up);
    vsg::dvec3 centerEyeSpace = (lv * _lookAt->center);

    vsg::dmat4 matrix = vsg::inverse(lv) * vsg::translate(centerEyeSpace) * rotation * vsg::translate(-centerEyeSpace) * lv;

    _lookAt->up = vsg::normalize(matrix * (_lookAt->eye + _lookAt->up) - matrix * _lookAt->eye);
    _lookAt->center = matrix * _lookAt->center;
    _lookAt->eye = matrix * _lookAt->eye;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraCabineManipulator::rotate_view(const vsg::dvec2& delta)
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraCabineManipulator::zoom(double coeff)
{
    double new_fovy = _perspective->fieldOfViewY;

    new_fovy = new_fovy * coeff;
    if ((new_fovy > _settings.fovy_max) || (new_fovy < _settings.fovy_min))
        return;

    _perspective->fieldOfViewY = new_fovy;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraCabineManipulator::move(const vsg::dvec3 &delta)
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
