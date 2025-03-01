#ifndef CAMERA_ABSTRACT_H
#define CAMERA_ABSTRACT_H

#include <vsg/ui/Keyboard.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/maths/transform.h>
#include <vsg/app/Camera.h>
#include "settings.h"
#include "VehicleExterior.h"

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
        , _settings(settings) {}

    virtual void resetView() {}
    virtual void returnView() {}
    virtual void keyboardPressEvent(vsg::KeySymbol key, bool pressed) {}
    virtual void mouseButtonPressEvent(uint32_t button, vsg::ButtonMask button_mask, bool pressed) {}
    virtual void mouseWheelEvent(vsg::vec3 delta) {}
    virtual void mouseMoveEvent(vsg::ButtonMask button_mask, vsg::dvec2 delta) {}
    virtual void touchZoomEvent(double zoomLevel) {}
    virtual void frameEvent(double dt) {}

    void setCurrentVehicle(VehicleExterior *cv) {_current_vehicle = cv;}

    vsg::ButtonMask getTouchToButtonMask() {return touchToButtonMask;}

protected:

    /// Button mask value used for touch events
    vsg::ButtonMask touchToButtonMask = vsg::BUTTON_MASK_3;

    vsg::ref_ptr<vsg::Keyboard> _keyboard = nullptr;

    vsg::ref_ptr<vsg::Camera> _camera = nullptr;
    vsg::ref_ptr<vsg::LookAt> _lookAt = nullptr;
    vsg::ref_ptr<vsg::Perspective> _perspective = nullptr;

    settings_t& _settings;

    VehicleExterior *_current_vehicle = nullptr;
};

#endif // CAMERA_CABINE_MANIPULATOR_H
