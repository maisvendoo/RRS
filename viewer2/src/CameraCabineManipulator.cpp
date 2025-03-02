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
void CameraCabineManipulator::resetView()
{
    if (!is_reset)
    {
        is_reset = true;

        _current_vehicle->saved_cabine_cam_shift = _position_shift;
        _current_vehicle->saved_cabine_cam_right = _angle_right;
        _current_vehicle->saved_cabine_cam_up = _angle_up;
        _current_vehicle->saved_cabine_cam_fov = _perspective->fieldOfViewY;
    }

    _position_shift = {0.0, 0.0, 0.0};
    _angle_right = 0.0;
    _angle_up = 0.0;
    _perspective->fieldOfViewY = _settings.fovy;

    calc_view();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraCabineManipulator::returnView()
{
    if (!is_reset)
    {
        resetView();
        return;
    }

    is_reset = false;

    _position_shift = _current_vehicle->saved_cabine_cam_shift;
    _angle_right = _current_vehicle->saved_cabine_cam_right;
    _angle_up = _current_vehicle->saved_cabine_cam_up;
    _perspective->fieldOfViewY = _current_vehicle->saved_cabine_cam_fov;

    calc_view();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraCabineManipulator::mouseWheelEvent(vsg::vec3 delta)
{
    if (_keyboard->pressed(vsg::KEY_Control_L) || _keyboard->pressed(vsg::KEY_Control_R))
    {
        if (delta.y > 0.0)
            move(vsg::dvec3(0.0, 0.0, _settings.cabine_height_step));
        else if (delta.y < 0.0)
            move(vsg::dvec3(0.0, 0.0, -_settings.cabine_height_step));
        return;
    }

    if (delta.y > 0.0)
        zoom(1 / _settings.cabine_fovy_coeff);
    else if (delta.y < 0.0)
        zoom(_settings.cabine_fovy_coeff);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraCabineManipulator::mouseMoveEvent(vsg::ButtonMask button_mask, vsg::dvec2 delta)
{
    if (button_mask & moveButtonMask)
    {
        vsg::dvec3 move_delta(-delta.x, delta.y, 0.0);

        if (move_delta)
            move(move_delta * _cameraMoveCoeff * _settings.cabine_speed_mouse);

        return;
    }

    if (button_mask & rotateButtonMask)
    {
        if (delta)
            rotate_view(-delta * (20.0 + _perspective->fieldOfViewY) * _settings.cabine_rotate_mouse);
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

    if (_keyboard->pressed(vsg::KEY_Alt_L) || _keyboard->pressed(vsg::KEY_Alt_R))
    {
        vsg::dvec2 rot_speed(0.0, 0.0);
        if ((speed = times2speed(_keyboard->times(turnRightKey))) != 0.0) rot_speed.x += -speed;
        if ((speed = times2speed(_keyboard->times(turnLeftKey))) != 0.0) rot_speed.x += speed;
        if ((speed = times2speed(_keyboard->times(pitchUpKey))) != 0.0) rot_speed.y += speed;
        if ((speed = times2speed(_keyboard->times(pitchDownKey))) != 0.0) rot_speed.y += -speed;

        if (rot_speed)
            rotate_view(rot_speed * dt * (20.0 + _perspective->fieldOfViewY) * _settings.cabine_rotate_keyboard);
        else
            calc_view();

        return;
    }

    vsg::dvec3 move_speed(0.0, 0.0, 0.0);
    if ((speed = times2speed(_keyboard->times(moveRightKey))) != 0.0) move_speed.x += speed;
    if ((speed = times2speed(_keyboard->times(moveLeftKey))) != 0.0) move_speed.x += -speed;
    if ((speed = times2speed(_keyboard->times(moveForwardKey))) != 0.0) move_speed.y += speed;
    if ((speed = times2speed(_keyboard->times(moveBackwardKey))) != 0.0) move_speed.y += -speed;
    //if ((speed = times2speed(_keyboard->times(moveUpKey))) != 0.0) move_speed.z += speed;
    //if ((speed = times2speed(_keyboard->times(moveDownKey))) != 0.0) move_speed.z += -speed;

    if (move_speed)
    {
        bool isCtrl = (_keyboard->pressed(vsg::KEY_Control_L) || _keyboard->pressed(vsg::KEY_Control_R));
        if (!_prevCtrl && isCtrl)
            _cameraMoveCoeff = _cameraMoveCoeff / _settings.cabine_speed_coeff;
        _prevCtrl = isCtrl;

        bool isShift = (_keyboard->pressed(vsg::KEY_Shift_L) || _keyboard->pressed(vsg::KEY_Shift_R));
        if (!_prevShift && isShift)
            _cameraMoveCoeff = _cameraMoveCoeff * _settings.cabine_speed_coeff;
        _prevShift = isShift;

        move(move_speed * dt * _cameraMoveCoeff * _settings.cabine_speed_keyboard);
    }
    else
    {
        _cameraMoveCoeff = 1.0;
        _prevCtrl = false;
        _prevShift = false;
        calc_view();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraCabineManipulator::rotate_view(const vsg::dvec2& delta)
{
    _angle_right += delta.x;
    if (_angle_right > vsg::PI) _angle_right += -2.0 * vsg::PI;
    if (_angle_right < -vsg::PI) _angle_right += 2.0 * vsg::PI;

    _angle_up += delta.y;
    _angle_up = std::max(_pitch_min, std::min(_pitch_max, _angle_up));

    if ((abs(_angle_right) > 1e-5) || (abs(_angle_up) > 1e-5))
        is_reset = false;

    calc_view();
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

    if (abs(new_fovy - _settings.fovy) > 5.0)
        is_reset = false;

    _perspective->fieldOfViewY = new_fovy;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraCabineManipulator::move(const vsg::dvec3 &delta)
{
    _position_shift += delta;
    _position_shift.z = std::max(_settings.cabine_z_min, std::min(_settings.cabine_z_max, _position_shift.z));

    if (length2(_position_shift) > 1e-5)
        is_reset = false;

    calc_view();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraCabineManipulator::calc_view()
{
    if (!_current_vehicle)
        return;

    vsg::dvec3 local_eye_pos = _current_vehicle->driver_pos + _position_shift;

    _lookAt->eye = _current_vehicle->position +
                   _current_vehicle->right * local_eye_pos.x +
                   _current_vehicle->orth * local_eye_pos.y +
                   _current_vehicle->up * local_eye_pos.z;

    _lookAt->center = _lookAt->eye + _current_vehicle->orth;
    _lookAt->up = _current_vehicle->up;

    if ((abs(_angle_right) > 1e-5) || (abs(_angle_up) > 1e-5))
    {
        vsg::dmat4 matrix = vsg::translate(_lookAt->eye) *
                            vsg::rotate(_angle_right, _current_vehicle->up) *
                            vsg::rotate(_angle_up, _current_vehicle->right) *
                            vsg::translate(-_lookAt->eye);

        _lookAt->up = vsg::normalize(matrix * (_lookAt->eye + _lookAt->up) - matrix * _lookAt->eye);
        _lookAt->center = matrix * _lookAt->center;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraCabineManipulator::currentVehicleChanged()
{
    if (_prev_current_vehicle == _current_vehicle)
        return;

    if (_prev_current_vehicle)
    {
        _prev_current_vehicle->saved_cabine_cam_shift = _position_shift;
        _prev_current_vehicle->saved_cabine_cam_right = _angle_right;
        _prev_current_vehicle->saved_cabine_cam_up = _angle_up;
        _prev_current_vehicle->saved_cabine_cam_fov = _perspective->fieldOfViewY;
    }

    if (_current_vehicle)
    {
        _prev_current_vehicle = _current_vehicle;

        is_reset = true;
        returnView();
    }
}
