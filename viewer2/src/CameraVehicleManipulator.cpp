#include "CameraVehicleManipulator.h"

CameraVehicleManipulator::CameraVehicleManipulator(vsg::ref_ptr<vsg::Keyboard> keyboard,
                                             vsg::ref_ptr<vsg::Camera> camera,
                                             settings_t &settings)
    : CameraAbstract(keyboard, camera, settings)
{
    _pitch_min = vsg::radians(_settings.pitch_min);
    _pitch_max = vsg::radians(_settings.pitch_max);
    _default_right = vsg::radians(_settings.ext_cam_init_angle_H);
    _default_up = -vsg::radians(_settings.ext_cam_init_angle_V);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraVehicleManipulator::resetView()
{
    _perspective->fieldOfViewY = _settings.fovy;

    if (!is_reset)
    {
        is_reset = true;

        _last_position_shift = _position_shift;
        _last_angle_right = _angle_right;
        _last_angle_up = _angle_up;
        _last_distance = _distance;
    }

    _position_shift = _settings.ext_cam_init_pos;
    _angle_right = _default_right;
    _angle_up = _default_up;
    _distance = _settings.ext_cam_init_distance;

    calc_view();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraVehicleManipulator::returnView()
{
    if (!is_reset)
    {
        resetView();
        return;
    }
    _perspective->fieldOfViewY = _settings.fovy;

    is_reset = false;

    _position_shift = _last_position_shift;
    _angle_right = _last_angle_right;
    _angle_up = _last_angle_up;
    _distance = _last_distance;

    calc_view();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraVehicleManipulator::mouseWheelEvent(vsg::vec3 delta)
{
    if (_keyboard->pressed(vsg::KEY_Control_L) || _keyboard->pressed(vsg::KEY_Control_R))
    {
        if (delta.y > 0.0)
            move(vsg::dvec3(0.0, 0.0, _settings.ext_cam_height_step));
        else if (delta.y < 0.0)
            move(vsg::dvec3(0.0, 0.0, -_settings.ext_cam_height_step));
        return;
    }

    if (delta.y > 0.0)
        zoom(1 / _settings.ext_cam_dist_coeff);
    else if (delta.y < 0.0)
        zoom(_settings.ext_cam_dist_coeff);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraVehicleManipulator::mouseMoveEvent(vsg::ButtonMask button_mask, vsg::dvec2 delta)
{
    if (button_mask & moveButtonMask)
    {
        vsg::dvec3 move_delta(-delta.x, delta.y, 0.0);

        if (move_delta)
            move(move_delta * _centerMoveCoeff * _settings.ext_cam_speed_mouse);

        return;
    }

    if (button_mask & rotateButtonMask)
    {
        if (delta)
            rotate_around(-delta * _settings.ext_cam_rotate_mouse);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraVehicleManipulator::touchZoomEvent(double zoomLevel)
{
    zoom(zoomLevel);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraVehicleManipulator::frameEvent(double dt)
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
            rotate_around(rot_speed * dt * _settings.ext_cam_rotate_keyboard);
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
            _centerMoveCoeff = _centerMoveCoeff / _settings.ext_cam_speed_coeff;
        _prevCtrl = isCtrl;

        bool isShift = (_keyboard->pressed(vsg::KEY_Shift_L) || _keyboard->pressed(vsg::KEY_Shift_R));
        if (!_prevShift && isShift)
            _centerMoveCoeff = _centerMoveCoeff * _settings.ext_cam_speed_coeff;
        _prevShift = isShift;

        move(move_speed * dt * _centerMoveCoeff * _settings.ext_cam_speed_keyboard);
    }
    else
    {
        _centerMoveCoeff = 1.0;
        _prevCtrl = false;
        _prevShift = false;
        calc_view();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraVehicleManipulator::rotate_around(const vsg::dvec2 &delta)
{
    is_reset = false;

    _angle_right += delta.x;
    if (_angle_right > vsg::PI) _angle_right += -2.0 * vsg::PI;
    if (_angle_right < -vsg::PI) _angle_right += 2.0 * vsg::PI;

    _angle_up += delta.y;
    _angle_up = std::max(_pitch_min, std::min(_pitch_max, _angle_up));

    calc_view();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraVehicleManipulator::zoom(double coeff)
{
    is_reset = false;

    double new_distance = _distance * coeff;
    if (new_distance < _settings.ext_cam_dist_min)
        return;

    _distance = new_distance;

    calc_view();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraVehicleManipulator::move(const vsg::dvec3 &delta)
{
    is_reset = false;

    _position_shift += delta;

    calc_view();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraVehicleManipulator::calc_view()
{
    if (!_current_vehicle)
        return;

    _lookAt->center = _current_vehicle->position +
                      _current_vehicle->right * _position_shift.x +
                      _current_vehicle->orth * _position_shift.y +
                      _current_vehicle->up * _position_shift.z;

    vsg::dvec3 view_direction = vsg::rotate(_angle_right, _current_vehicle->up) *
                                _current_vehicle->orth *
                                (_distance * _current_vehicle->orientation);
    vsg::dvec3 view_right_axis = normalize(vsg::cross(-view_direction, _current_vehicle->up));
    vsg::dmat4 matrix = vsg::translate(_lookAt->center) *
                        vsg::rotate(_angle_up, view_right_axis) *
                        vsg::translate(-_lookAt->center);

    _lookAt->eye = _lookAt->center + view_direction;
    _lookAt->up = vsg::normalize(matrix * (_lookAt->eye + _current_vehicle->up) - matrix * _lookAt->eye);
    _lookAt->eye = matrix * (_lookAt->eye);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraVehicleManipulator::currentVehicleChanged()
{
    calc_view();
}
