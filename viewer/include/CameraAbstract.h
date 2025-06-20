#ifndef CAMERA_ABSTRACT_H
#define CAMERA_ABSTRACT_H

#include "settings.h"
#include "VehicleExterior.h"

#include <vsg/app/Camera.h>
#include <vsg/ui/Keyboard.h>
#include <vsg/ui/PointerEvent.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class CameraAbstract
{
public:
    CameraAbstract(vsg::ref_ptr<vsg::Keyboard> keyboard,
                   vsg::ref_ptr<vsg::Camera> camera,
                   settings_t &settings)
        : _keyboard(keyboard)
        , _camera(camera)
        , _lookAt(camera->viewMatrix.cast<vsg::LookAt>())
        , _perspective(camera->projectionMatrix.cast<vsg::Perspective>())
        , _settings(settings)
    {
    }

    virtual void resetView() = 0;
    virtual void returnView() = 0;

    virtual void keyboardPressEvent([[maybe_unused]] vsg::KeySymbol key, [[maybe_unused]] bool pressed) {}
    virtual void mouseButtonPressEvent([[maybe_unused]] uint32_t button, [[maybe_unused]] vsg::ButtonMask button_mask, [[maybe_unused]] bool pressed) {}

    virtual void mouseWheelEvent(vsg::vec3 delta) = 0;
    virtual void mouseMoveEvent(vsg::ButtonMask button_mask, vsg::dvec2 delta) = 0;
    virtual void touchZoomEvent(double zoomLevel) = 0;
    virtual void frameEvent(double dt) = 0;

    void setCurrentVehicle(VehicleExterior* current_vehicle)
    {
        _current_vehicle = current_vehicle;
        currentVehicleChanged();
    }

    vsg::ButtonMask getTouchToButtonMask() const { return touchToButtonMask; }

protected:
    virtual void currentVehicleChanged() {}

    /// Button mask value used for touch events
    vsg::ButtonMask touchToButtonMask = vsg::BUTTON_MASK_3;

    vsg::ref_ptr<vsg::Keyboard> _keyboard = nullptr;

    vsg::ref_ptr<vsg::Camera> _camera = nullptr;
    vsg::ref_ptr<vsg::LookAt> _lookAt = nullptr;
    vsg::ref_ptr<vsg::Perspective> _perspective = nullptr;

    settings_t& _settings;

    VehicleExterior* _current_vehicle = nullptr;
};

#endif // CAMERA_CABINE_MANIPULATOR_H
