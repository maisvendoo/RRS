#pragma once
#ifndef UPDATE_VIEWER_HANDLER_H
#define UPDATE_VIEWER_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/Visitor.h>
#include <vsg/maths/vec2.h>

#include <cstdint>
#include <map>
#include <utility>

class CameraAbstract;
class ScreenshotWriter;
struct settings_t;
class TrafficLightsHandler;
class UpdateControlToServerHandler;
class VehiclesHandler;

namespace vsg
{
    class ButtonPressEvent;
    class ButtonReleaseEvent;
    class Camera;
    class FocusInEvent;
    class FocusOutEvent;
    class FrameEvent;
    class Keyboard;
    class KeyPressEvent;
    class KeyReleaseEvent;
    class MoveEvent;
    class PointerEvent;
    class RegionOfInterest;
    class ScrollWheelEvent;
    class TouchDownEvent;
    class TouchEvent;
    class TouchMoveEvent;
    class TouchUpEvent;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class UpdateViewerHandler final : public vsg::Inherit<vsg::Visitor, UpdateViewerHandler>
{
public:
    UpdateViewerHandler(
        vsg::ref_ptr<UpdateControlToServerHandler> upd_server_control,
        vsg::ref_ptr<vsg::Camera> camera,
        vsg::ref_ptr<vsg::RegionOfInterest> shadow_region,
        ScreenshotWriter* screenshot_writer,
        TrafficLightsHandler* sig_handler,
        VehiclesHandler* veh_handler,
        settings_t& settings
    );

    ~UpdateViewerHandler() noexcept;

    void apply(vsg::FrameEvent& frame) override;
    void apply(vsg::KeyPressEvent& keyPress) override;    
    void apply(vsg::KeyReleaseEvent& keyRelease) override;
    void apply(vsg::FocusInEvent& focusIn) override;
    void apply(vsg::FocusOutEvent& focusOut) override;
    void apply(vsg::ButtonPressEvent& buttonPress) override;
    void apply(vsg::ButtonReleaseEvent& buttonRelease) override;
    void apply(vsg::MoveEvent& moveEvent) override;
    void apply(vsg::ScrollWheelEvent& scrollWheel) override;
    void apply(vsg::TouchDownEvent& touchDown) override;
    void apply(vsg::TouchUpEvent& touchUp) override;
    void apply(vsg::TouchMoveEvent& touchMove) override;

    void changeCurrentVehicle();

private:
    /// compute non-dimensional window coordinate (-1, 1) from event coords
    vsg::dvec2 ndc(const vsg::PointerEvent& event) const;

    std::pair<std::int32_t, std::int32_t> cameraRenderAreaCoordinates(const vsg::PointerEvent& pointerEvent) const;

    bool withinRenderArea(const vsg::PointerEvent& pointerEvent) const;

    bool isAlt() const;
    bool isCtrl() const;
    bool isShift() const;


    void changeCurrentCabine();

    void updateShadowRegion();

    settings_t& _settings;
    vsg::ref_ptr<vsg::Keyboard> _keyboard;
    vsg::ref_ptr<UpdateControlToServerHandler> _upd_server_control;
    vsg::ref_ptr<vsg::Camera> _camera;
    vsg::ref_ptr<vsg::RegionOfInterest> _shadow_region;

    bool _hasKeyboardFocus = false;
    bool _hasPointerFocus = false;
    bool _lastPointerEventWithinRenderArea = false;
    double _previousTime = 0.0;
    vsg::ref_ptr<vsg::PointerEvent> _previousPointerEvent;
    std::map<std::uint32_t, vsg::ref_ptr<vsg::TouchEvent>> _previousTouches;
    double _prevZoomTouchDistance = 0.0;

    CameraAbstract* _current_manipulator = nullptr;

    CameraAbstract* _free_manipulator = nullptr;
    CameraAbstract* _vehicle_manipulator = nullptr;
    CameraAbstract* _cabine_manipulator = nullptr;
    CameraAbstract* _follow_manipulator = nullptr;

    ScreenshotWriter* _screenshot_writer = nullptr;
    TrafficLightsHandler* _sig_handler = nullptr;
    VehiclesHandler* _vehicles_handler = nullptr;

    bool _wasPausePhysicallyPressed = false;
    void setPause();
};

#endif // UPDATE_VIEWER_HANDLER_H
