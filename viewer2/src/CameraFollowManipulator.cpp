#include "CameraFollowManipulator.h"

CameraFollowManipulator::CameraFollowManipulator(vsg::ref_ptr<vsg::Keyboard> keyboard,
                                                 vsg::ref_ptr<vsg::Camera> camera,
                                                 settings_t &settings)
    : CameraAbstract(keyboard, camera, settings)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraFollowManipulator::resetView()
{
    // В этом методе инициализируем положение камеры справа
    _is_right = true;
    set_view();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraFollowManipulator::returnView()
{
    // В этом методе инициализируем положение камеры слева
    _is_right = false;
    set_view();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraFollowManipulator::mouseWheelEvent(vsg::vec3 delta)
{
    if (_keyboard->pressed(vsg::KEY_Control_L) || _keyboard->pressed(vsg::KEY_Control_R))
    {
        if (delta.y > 0.0)
            move(vsg::dvec3(0.0, 0.0, _settings.follow_cam_height_step));
        else if (delta.y < 0.0)
            move(vsg::dvec3(0.0, 0.0, -_settings.follow_cam_height_step));
        return;
    }

    if (delta.y > 0.0)
        zoom(1 / _settings.follow_cam_fovy_coeff);
    else if (delta.y < 0.0)
        zoom(_settings.follow_cam_fovy_coeff);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraFollowManipulator::mouseMoveEvent(vsg::ButtonMask button_mask, vsg::dvec2 delta)
{
    if ((button_mask & moveButtonMask) || (button_mask & rotateButtonMask))
    {
        vsg::dvec3 move_delta(-delta.x, delta.y, 0.0);

        if (move_delta)
            move(move_delta * _centerMoveCoeff * _settings.follow_cam_speed_mouse);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraFollowManipulator::touchZoomEvent(double zoomLevel)
{
    zoom(zoomLevel);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraFollowManipulator::frameEvent(double dt)
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
            _centerMoveCoeff = _centerMoveCoeff / _settings.follow_cam_speed_coeff;
        _prevCtrl = isCtrl;

        bool isShift = (_keyboard->pressed(vsg::KEY_Shift_L) || _keyboard->pressed(vsg::KEY_Shift_R));
        if (!_prevShift && isShift)
            _centerMoveCoeff = _centerMoveCoeff * _settings.follow_cam_speed_coeff;
        _prevShift = isShift;

        move(move_speed * dt * _centerMoveCoeff * _settings.follow_cam_speed_keyboard);
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
void CameraFollowManipulator::zoom(double coeff)
{
    double new_fovy = _perspective->fieldOfViewY * coeff;
    if ((new_fovy > _settings.fovy_max) || (new_fovy < _settings.fovy_min))
        return;

    _perspective->fieldOfViewY = new_fovy;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraFollowManipulator::move(const vsg::dvec3 &delta)
{
    vsg::dvec3 center(0.0, 0.0, 0.0);
    vsg::dvec3 up(0.0, 0.0, 1.0);
    if (_current_vehicle)
    {
        center = _current_vehicle->position;
        up = _current_vehicle->up;
    }

    vsg::dvec3 view = center - _lookAt->eye;

    // Check there is no divide by zero
    if (length2(view) < 1.0)
    {
        set_view();
        return;
    }

    vsg::dvec3 right = cross(vsg::normalize(view), up);
    view = cross(up, right);

    _lookAt->eye += right * delta.x;
    _lookAt->eye += view * delta.y;
    _lookAt->eye += up * delta.z;

    calc_view();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraFollowManipulator::set_view()
{
    _perspective->fieldOfViewY = _settings.fovy;

    vsg::dvec3 shift(_settings.follow_cam_init_shift_right * (_is_right ? 1.0 : -1.0),
                     _settings.follow_cam_fwd_velocity_coeff,
                     _settings.follow_cam_init_shift_up);
    if (_current_vehicle)
    {
        vsg::dvec3 v = _current_vehicle->velocity;
        v += vsg::dvec3(cbrt(v.x), cbrt(v.y), cbrt(v.z)) * 5.0;

        shift = _current_vehicle->position +
                _current_vehicle->right * shift.x +
                v * shift.y +
                _current_vehicle->up * shift.z;
    }

    _lookAt->eye = shift;
    calc_view();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraFollowManipulator::calc_view()
{
    _lookAt->center = {0.0, 0.0, 0.0};
    _lookAt->up = {0.0, 0.0, 1.0};
    if (_current_vehicle)
    {
        _lookAt->center = _current_vehicle->position +
            _current_vehicle->up * _settings.follow_cam_init_shift_up;
        _lookAt->up = _current_vehicle->up;
    }

    vsg::dvec3 view = _lookAt->center - _lookAt->eye;

    // Check there is no divide by zero
    if (length2(view) < 1.0)
    {
        set_view();
        return;
    }

    view = vsg::normalize(view);
    vsg::dvec3 right(vsg::cross(view, _lookAt->up));
    _lookAt->up = vsg::cross(right, view);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraFollowManipulator::currentVehicleChanged()
{
    if (_prev_train_id == _current_vehicle->train_id)
    {
        calc_view();
        return;
    }

    _prev_train_id = _current_vehicle->train_id;
    set_view();
}
