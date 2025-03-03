#ifndef CAMERA_FOLLOW_MANIPULATOR_H
#define CAMERA_FOLLOW_MANIPULATOR_H

#include "CameraAbstract.h"

class CameraFollowManipulator : public CameraAbstract
{
public:
    CameraFollowManipulator(vsg::ref_ptr<vsg::Keyboard> keyboard,
                            vsg::ref_ptr<vsg::Camera> camera,
                            settings_t &settings);

    void resetView() override;
    void returnView() override;
    void mouseWheelEvent(vsg::vec3 delta) override;
    void mouseMoveEvent(vsg::ButtonMask button_mask, vsg::dvec2 delta) override;
    void touchZoomEvent(double zoomLevel) override;
    void frameEvent(double dt) override;

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

    void zoom(double coeff);
    void move(const vsg::dvec3& delta);
    void set_view();
    void calc_view();

    void currentVehicleChanged() override;

    bool _prevCtrl = false;
    bool _prevShift = false;
    double _centerMoveCoeff = 1.0;

    bool _is_right = true;
    int _prev_train_id = -1;
};

#endif // CAMERA_FOLLOW_MANIPULATOR_H
