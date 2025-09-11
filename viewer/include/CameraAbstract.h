#ifndef CAMERA_ABSTRACT_H
#define CAMERA_ABSTRACT_H

#include <vsg/app/Camera.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/Keyboard.h>

#include <cstdint>

class VehicleExterior;
struct settings_t;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class CameraAbstract
{
public:
    CameraAbstract(
        vsg::ref_ptr<vsg::Keyboard> keyboard,
        vsg::ref_ptr<vsg::Camera> camera,
        const settings_t& settings
    );

    virtual ~CameraAbstract() = default;

    virtual void resetView() = 0;
    virtual void returnView() = 0;

    virtual void keyboardPressEvent(vsg::KeySymbol key, bool pressed);
    virtual void mouseButtonPressEvent(std::uint32_t button, vsg::ButtonMask button_mask, bool pressed);

    virtual void mouseWheelEvent(vsg::vec3 delta) = 0;
    virtual void mouseMoveEvent(vsg::ButtonMask button_mask, vsg::dvec2 delta) = 0;
    virtual void touchZoomEvent(double zoomLevel) = 0;
    virtual void frameEvent(double dt) = 0;

    void setCurrentVehicle(VehicleExterior* current_vehicle);

    vsg::ButtonMask getTouchToButtonMask() const;

protected:
    virtual void currentVehicleChanged();

    /// Button mask value used for touch events
    vsg::ButtonMask touchToButtonMask = vsg::BUTTON_MASK_3;

    vsg::ref_ptr<vsg::Keyboard> _keyboard;

    vsg::ref_ptr<vsg::Camera> _camera;
    vsg::ref_ptr<vsg::LookAt> _lookAt;
    vsg::ref_ptr<vsg::Perspective> _perspective;

    const settings_t& _settings;

    VehicleExterior* _current_vehicle = nullptr;
};

#endif // CAMERA_CABINE_MANIPULATOR_H
