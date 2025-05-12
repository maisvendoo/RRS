#ifndef UPDATE_VIEWER_HANDLER_H
#define UPDATE_VIEWER_HANDLER_H

#include "settings.h"
#include "CameraAbstract.h"
#include "UpdateControlToServerHandler.h"

#include <vsg/nodes/RegionOfInterest.h>

class CameraFreeManipulator;
class CameraVehicleManipulator;
class CameraCabineManipulator;
class CameraFollowManipulator;
class ScreenshotWriter;
class TrafficLightsHandler;
class VehiclesHandler;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class UpdateViewerHandler : public vsg::Inherit<vsg::Visitor, UpdateViewerHandler>
{
public:
    explicit UpdateViewerHandler(vsg::ref_ptr<UpdateControlToServerHandler> upd_server_control,
                                 vsg::ref_ptr<vsg::Camera> camera,
                                 vsg::ref_ptr<vsg::RegionOfInterest> shadow_region,
                                 ScreenshotWriter *screenshot_writer,
                                 TrafficLightsHandler *sig_handler,
                                 VehiclesHandler *veh_handler,
                                 settings_t &settings);

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

private:

    /// compute non dimensional window coordinate (-1,1) from event coords
    vsg::dvec2 ndc(const vsg::PointerEvent& event);

    std::pair<int32_t, int32_t> cameraRenderAreaCoordinates(const vsg::PointerEvent& pointerEvent) const;

    bool withinRenderArea(const vsg::PointerEvent& pointerEvent) const;

    bool isAlt();
    bool isCtrl();
    bool isShift();

    void changeCurrentVehicle();

    void updateShadowRegion();

    settings_t& _settings;
    vsg::ref_ptr<vsg::Keyboard> _keyboard = nullptr;
    vsg::ref_ptr<UpdateControlToServerHandler> _upd_server_control = nullptr;
    vsg::ref_ptr<vsg::Camera> _camera = nullptr;
    vsg::ref_ptr<vsg::RegionOfInterest> _shadow_region = nullptr;

    bool _hasKeyboardFocus = false;
    bool _hasPointerFocus = false;
    bool _lastPointerEventWithinRenderArea = false;
    double _previousTime = 0.0;
    vsg::ref_ptr<vsg::PointerEvent> _previousPointerEvent;
    std::map<uint32_t, vsg::ref_ptr<vsg::TouchEvent>> _previousTouches;
    double _prevZoomTouchDistance = 0.0;

    CameraAbstract *_current_manipulator = nullptr;

    CameraFreeManipulator *_free_manipulator = nullptr;
    CameraVehicleManipulator *_vehicle_manipulator = nullptr;
    CameraCabineManipulator *_cabine_manipulator = nullptr;
    CameraFollowManipulator *_follow_manipulator = nullptr;
    ScreenshotWriter *_screenshot_writer = nullptr;
    TrafficLightsHandler *_sig_handler = nullptr;
    VehiclesHandler *_vehicles_handler = nullptr;
};

#endif // UPDATE_VIEWER_HANDLER_H
