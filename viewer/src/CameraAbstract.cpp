#include "CameraAbstract.h"

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

CameraAbstract::CameraAbstract(
    vsg::ref_ptr<vsg::Keyboard> keyboard,
    vsg::ref_ptr<vsg::Camera> camera,
    const settings_t& settings
)
    : _keyboard(keyboard)
    , _camera(camera)
    , _lookAt(camera->viewMatrix.cast<vsg::LookAt>())
    , _perspective(camera->projectionMatrix.cast<vsg::Perspective>())
    , _settings(settings)
{
}

void CameraAbstract::keyboardPressEvent(vsg::KeySymbol key, bool pressed)
{
    (void)key;
    (void)pressed;
}

void CameraAbstract::mouseButtonPressEvent(std::uint32_t button, vsg::ButtonMask button_mask, bool pressed)
{
    (void)button;
    (void)button_mask;
    (void)pressed;
}

void CameraAbstract::setCurrentVehicle(VehicleExterior* current_vehicle)
{
    _current_vehicle = current_vehicle;
    currentVehicleChanged();
}

vsg::ButtonMask CameraAbstract::getTouchToButtonMask() const
{
    return touchToButtonMask;
}

void CameraAbstract::currentVehicleChanged()
{
}
