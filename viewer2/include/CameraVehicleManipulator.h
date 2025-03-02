#ifndef CAMERA_VEHICLE_MANIPULATOR_H
#define CAMERA_VEHICLE_MANIPULATOR_H

#include "CameraAbstract.h"

class CameraVehicleManipulator : public CameraAbstract
{
public:
    CameraVehicleManipulator(vsg::ref_ptr<vsg::Keyboard> keyboard,
                             vsg::ref_ptr<vsg::Camera> camera,
                             settings_t &settings);

    void resetView() override;
    void returnView() override;
    void mouseWheelEvent(vsg::vec3 delta) override;
    void mouseMoveEvent(vsg::ButtonMask button_mask, vsg::dvec2 delta) override;
    void touchZoomEvent(double zoomLevel) override;
    void frameEvent(double dt) override;

    /// Key that turns the view left around the center points
    vsg::KeySymbol turnLeftKey = vsg::KEY_Left;

    /// Key that turns the view right around the center points
    vsg::KeySymbol turnRightKey = vsg::KEY_Right;

    /// Key that pitches up the view around the center point
    vsg::KeySymbol pitchUpKey = vsg::KEY_Up;

    /// Key that pitches down the view around the center point
    vsg::KeySymbol pitchDownKey = vsg::KEY_Down;

    /// Key that moves the center forward
    vsg::KeySymbol moveForwardKey = vsg::KEY_Up;

    /// Key that moves the center backwards
    vsg::KeySymbol moveBackwardKey = vsg::KEY_Down;

    /// Key that moves the center left
    vsg::KeySymbol moveLeftKey = vsg::KEY_Left;

    /// Key that moves the center right
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

private:

    void rotate_around(const vsg::dvec2& delta);
    void zoom(double coeff);
    void move(const vsg::dvec3& delta);
    void calc_view();

    void currentVehicleChanged() override;

    bool _prevCtrl = false;
    bool _prevShift = false;
    double _centerMoveCoeff = 1.0;
    double _pitch_min = vsg::radians(-70.0);
    double _pitch_max = vsg::radians(70.0);
    double _default_right = vsg::radians(-45.0);
    double _default_up = -vsg::radians(10.0);

    vsg::dvec3 _position_shift = {0.0, 0.0, 0.0};
    double _distance = 50.0;
    double _angle_right = 0.0;
    double _angle_up = 0.0;

    bool is_reset = true;
    vsg::dvec3 _last_position_shift = {0.0, 0.0, 0.0};
    double _last_angle_right = 0.0;
    double _last_angle_up = 0.0;
    double _last_distance = 16.0 / sqrt(2.0);
};

#endif // CAMERA_VEHICLE_MANIPULATOR_H
