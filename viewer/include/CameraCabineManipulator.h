#ifndef CAMERA_CABINE_MANIPULATOR_H
#define CAMERA_CABINE_MANIPULATOR_H

#include "CameraAbstract.h"
#include "CabSway.h"

class CameraCabineManipulator final : public CameraAbstract
{
public:
    CameraCabineManipulator(vsg::ref_ptr<vsg::Keyboard> keyboard,
                            vsg::ref_ptr<vsg::Camera> camera,
                            settings_t &settings);

    virtual void resetView() override;
    virtual void returnView() override;
    void mouseWheelEvent(vsg::vec3 delta) override;
    void mouseMoveEvent(vsg::ButtonMask button_mask, vsg::dvec2 delta) override;
    void touchZoomEvent(double zoomLevel) override;
    void frameEvent(double dt) override;

    /// Открыть/закрыть боковое окно кабины (Num0)
    void toggleWindow();

    /// Высунуться/вернуться (Num1). false - окно ещё закрыто
    bool toggleLeanOut();

    bool isWindowOpen() const noexcept { return _window_open; }
    bool isLeanOut() const noexcept { return _lean_out_target; }

    /// Key that turns the view left around the eye points
    vsg::KeySymbol turnLeftKey = vsg::KEY_Left;

    /// Key that turns the view right around the eye points
    vsg::KeySymbol turnRightKey = vsg::KEY_Right;

    /// Key that pitches up the view around the eye point
    vsg::KeySymbol pitchUpKey = vsg::KEY_Up;

    /// Key that pitches down the view around the eye point
    vsg::KeySymbol pitchDownKey = vsg::KEY_Down;

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
    vsg::ButtonMask rotateButtonMask = vsg::BUTTON_MASK_2;

    /// Button mask value used to moving camera horizontally, defaults to middle mouse button
    vsg::ButtonMask moveButtonMask = vsg::BUTTON_MASK_2;

private:

    void rotate_view(const vsg::dvec2& delta);
    void zoom(double coeff);
    void move(const vsg::dvec3& delta);
    void calc_view();

    void currentVehicleChanged() override;

    bool _prevCtrl = false;
    bool _prevShift = false;
    double _cameraMoveCoeff = 1.0;
    double _pitch_min = vsg::radians(-70.0);
    double _pitch_max = vsg::radians(70.0);

    vsg::dvec3 _position_shift = {0.0, 0.0, 0.0};
    double _angle_right = 0.0;
    double _angle_up = 0.0;

    bool is_reset = true;
    VehicleExterior *_prev_current_vehicle = nullptr;

    /// Раскачка кабины в движении (покачивание кузова на рессорах):
    /// дополняет физическую реакцию cam_motion, чисто визуальная
    CabSway _sway;

    bool is_shift = false;

    /// Боковое окно кабины и высунувшийся машинист:
    /// _lean_out плавно едет к _lean_out_target (0..1)
    bool _window_open = false;
    bool _lean_out_target = false;
    double _lean_out = 0.0;
};

#endif // CAMERA_CABINE_MANIPULATOR_H
