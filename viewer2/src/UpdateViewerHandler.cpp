#include "UpdateViewerHandler.h"

#include <vsg/ui/ScrollWheelEvent.h>
#include <vsg/ui/TouchEvent.h>

#include "CameraFreeManipulator.h"
#include "CameraVehicleManipulator.h"
#include "CameraCabineManipulator.h"
#include "CameraFollowManipulator.h"
#include "ScreenshotWriter.h"
#include "TrafficLightsHandler.h"
#include "VehiclesHandler.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
UpdateViewerHandler::UpdateViewerHandler(vsg::ref_ptr<UpdateControlToServerHandler> upd_server_control,
                                         vsg::ref_ptr<vsg::Camera> camera,
                                         vsg::ref_ptr<vsg::RegionOfInterest> shadow_region,
                                         ScreenshotWriter *screenshot_writer,
                                         TrafficLightsHandler *sig_handler,
                                         VehiclesHandler *veh_handler,
                                         settings_t &settings)
    : Inherit()
    , _settings(settings)
    , _keyboard(vsg::Keyboard::create())
    , _upd_server_control(upd_server_control)
    , _camera(camera)
    , _shadow_region(shadow_region)
    , _screenshot_writer(screenshot_writer)
    , _sig_handler(sig_handler)
    , _vehicles_handler(veh_handler)
{
    _free_manipulator = new CameraFreeManipulator(_keyboard, _camera, _settings);
    _vehicle_manipulator = new CameraVehicleManipulator(_keyboard, _camera, _settings);
    _cabine_manipulator = new CameraCabineManipulator(_keyboard, _camera, _settings);
    _follow_manipulator = new CameraFollowManipulator(_keyboard, _camera, _settings);

    _current_manipulator = _free_manipulator;
    _current_manipulator->resetView();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::apply(vsg::FrameEvent& frame)
{
    if (frame.frameStamp->frameCount)
    {
        double t = frame.frameStamp->simulationTime;
        double dt = t - _previousTime;
        _previousTime = t;

        _sig_handler->step(static_cast<float>(t), static_cast<float>(dt));

        _vehicles_handler->step(t, dt);

        _current_manipulator->frameEvent(dt);

        updateShadowRegion();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::apply(vsg::KeyPressEvent& keyPress)
{
    if (_keyboard) keyPress.accept(*_keyboard);

    _current_manipulator->keyboardPressEvent(keyPress.keyBase, true);

    // Пока не загрузились и не подключились к серверу - только свободная камера
    if (!_vehicles_handler->isUpdated())
    {
        if (keyPress.keyBase == vsg::KEY_F4)
        {
            _current_manipulator->returnView();
        }
        return;
    }

    // Запрещаем управление выбором текущего вагона со свободной камеры
    if (_current_manipulator != _free_manipulator)
    {
        // Home - первый вагон следующего поезда на сервере
        if (keyPress.keyBase == vsg::KEY_Home)
        {
            if (_vehicles_handler->selectNextTrain())
                changeCurrentVehicle();

            return;
        }

        // End - первый вагон предыдущего поезда на сервере
        if (keyPress.keyBase == vsg::KEY_End)
        {
            if (_vehicles_handler->selectPrevTrain())
                changeCurrentVehicle();

            return;
        }

        // Page Up - следующий вагон поезда
        if (keyPress.keyBase == vsg::KEY_Page_Up)
        {
            if (_vehicles_handler->selectNextVehicle())
                changeCurrentVehicle();

            return;
        }

        // Page Down - предыдущий вагон поезда
        if (keyPress.keyBase == vsg::KEY_Page_Down)
        {
            if (_vehicles_handler->selectPrevVehicle())
                changeCurrentVehicle();

            return;
        }

        // Enter - взять управление текущим вагоном
        if ((keyPress.keyBase == vsg::KEY_KP_Enter) || (keyPress.keyBase == vsg::KEY_Return))
        {
            _vehicles_handler->selectControlVehicle();
            changeCurrentVehicle();

            return;
        }
    }

    // Управление F-клавишами только без Shift и Ctrl
    if (!isCtrl() && !isShift())
    {
        // F1 - камера из кабины управляемой ПЕ
        if (keyPress.keyBase == vsg::KEY_F1)
        {
            if (_vehicles_handler->returnToControlledVehicle() || (_current_manipulator != _cabine_manipulator))
            {
                _current_manipulator = _cabine_manipulator;
                _current_manipulator->setCurrentVehicle(_vehicles_handler->getCurrentVehicle());
                _current_manipulator->resetView();
                return;
            }

            _current_manipulator->returnView();
            return;
        }

        // F2 - камера из кабины текущей ПЕ
        if (keyPress.keyBase == vsg::KEY_F2)
        {
            if (_current_manipulator == _cabine_manipulator)
            {
                _current_manipulator->returnView();
                return;
            }

            _current_manipulator = _cabine_manipulator;
            _current_manipulator->setCurrentVehicle(_vehicles_handler->getCurrentVehicle());
            _current_manipulator->resetView();
            return;
        }

        // F3 - внешняя камера текущей ПЕ
        if (keyPress.keyBase == vsg::KEY_F3)
        {
            if (_current_manipulator == _vehicle_manipulator)
            {
                _current_manipulator->returnView();
                return;
            }

            _current_manipulator = _vehicle_manipulator;
            _current_manipulator->setCurrentVehicle(_vehicles_handler->getCurrentVehicle());
            _current_manipulator->resetView();
            return;
        }

        // F4 - свободная камера
        if (keyPress.keyBase == vsg::KEY_F4)
        {
            if (_current_manipulator == _free_manipulator)
            {
                _current_manipulator->returnView();
                return;
            }

            _current_manipulator = _free_manipulator;
            _current_manipulator->setCurrentVehicle(_vehicles_handler->getCurrentVehicle());
            _current_manipulator->resetView();
            return;
        }

        // F5 - следящая камера справа по ходу поезда
        if (keyPress.keyBase == vsg::KEY_F5)
        {
            _current_manipulator = _follow_manipulator;
            _current_manipulator->setCurrentVehicle(_vehicles_handler->getCurrentVehicle());
            _current_manipulator->resetView(); // Камера справа инициализируется в resetView
            return;
        }

        // F6 - следящая камера слева по ходу поезда
        if (keyPress.keyBase == vsg::KEY_F6)
        {
            _current_manipulator = _follow_manipulator;
            _current_manipulator->setCurrentVehicle(_vehicles_handler->getCurrentVehicle());
            _current_manipulator->returnView(); // Камера слева инициализируется в returnView
            return;
        }

        // F12 - скриншот
        if (keyPress.keyBase == vsg::KEY_F12)
        {
            _screenshot_writer->setScreenshot();
            return;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    if (_keyboard) keyRelease.accept(*_keyboard);

    _current_manipulator->keyboardPressEvent(keyRelease.keyBase, false);

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::apply(vsg::FocusInEvent& focusIn)
{
    if (_keyboard) focusIn.accept(*_keyboard);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::apply(vsg::FocusOutEvent& focusOut)
{
    if (_keyboard) focusOut.accept(*_keyboard);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        _hasKeyboardFocus = false;
        return;
    }

    _hasPointerFocus = _hasKeyboardFocus = withinRenderArea(buttonPress);
    _lastPointerEventWithinRenderArea = _hasPointerFocus;

    if (_hasPointerFocus) buttonPress.handled = true;

    _current_manipulator->mouseButtonPressEvent(buttonPress.button, buttonPress.mask, true);

    _previousPointerEvent = &buttonPress;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    if (buttonRelease.handled) return;

    _lastPointerEventWithinRenderArea = withinRenderArea(buttonRelease);
    _hasPointerFocus = false;

    _current_manipulator->mouseButtonPressEvent(buttonRelease.button, buttonRelease.mask, false);

    _previousPointerEvent = &buttonRelease;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::apply(vsg::MoveEvent& moveEvent)
{
    _lastPointerEventWithinRenderArea = withinRenderArea(moveEvent);

    if (moveEvent.handled || !_hasPointerFocus) return;

    if (_previousPointerEvent)
    {
        vsg::dvec2 new_ndc = ndc(moveEvent);
        vsg::dvec2 prev_ndc = ndc(*_previousPointerEvent);
        _current_manipulator->mouseMoveEvent(moveEvent.mask, (new_ndc - prev_ndc));
    }

    _previousPointerEvent = &moveEvent;
    moveEvent.handled = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::apply(vsg::ScrollWheelEvent& scrollWheel)
{
    if (scrollWheel.handled || !_lastPointerEventWithinRenderArea) return;

    _current_manipulator->mouseWheelEvent(scrollWheel.delta);

    scrollWheel.handled = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::apply(vsg::TouchDownEvent& touchDown)
{
    _previousTouches[touchDown.id] = &touchDown;
    switch (touchDown.id)
    {
    case 0: {
        if (_previousTouches.size() == 1)
        {
            vsg::ref_ptr<vsg::Window> w = touchDown.window;
            vsg::ref_ptr<vsg::ButtonPressEvent> evt = vsg::ButtonPressEvent::create(
                w,
                touchDown.time,
                touchDown.x,
                touchDown.y,
                _current_manipulator->getTouchToButtonMask(),
                touchDown.id);
            apply(*evt.get());
        }
        return;
    }
    case 1: {
        _prevZoomTouchDistance = 0.0;
        if (touchDown.id == 0 && _previousTouches.count(1))
        {
            const auto& prevTouch1 = _previousTouches[1];
            auto a = std::abs(static_cast<double>(prevTouch1->x) - touchDown.x);
            auto b = std::abs(static_cast<double>(prevTouch1->y) - touchDown.y);
            if (a > 0 || b > 0)
                _prevZoomTouchDistance = sqrt(a * a + b * b);
        }
        return;
    }
    default: return;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::apply(vsg::TouchUpEvent& touchUp)
{
    if (touchUp.id == 0 && _previousTouches.size() == 1)
    {
        vsg::ref_ptr<vsg::Window> w = touchUp.window;
        vsg::ref_ptr<vsg::ButtonReleaseEvent> evt = vsg::ButtonReleaseEvent::create(
            w,
            touchUp.time,
            touchUp.x,
            touchUp.y,
            _current_manipulator->getTouchToButtonMask(),
            touchUp.id);
        apply(*evt.get());
    }
    _previousTouches.erase(touchUp.id);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::apply(vsg::TouchMoveEvent& touchMove)
{
    vsg::ref_ptr<vsg::Window> w = touchMove.window;
    switch (_previousTouches.size())
    {
    case 1: {
        // Rotate
        vsg::ref_ptr<vsg::MoveEvent> evt = vsg::MoveEvent::create(
            w,
            touchMove.time,
            touchMove.x,
            touchMove.y,
            _current_manipulator->getTouchToButtonMask());
        apply(*evt.get());
        break;
    }
    case 2: {
        if (touchMove.id == 0 && _previousTouches.count(0))
        {
            // Zoom
            const auto& prevTouch1 = _previousTouches[1];
            auto a = std::abs(static_cast<double>(prevTouch1->x) - touchMove.x);
            auto b = std::abs(static_cast<double>(prevTouch1->y) - touchMove.y);
            if (a > 0 || b > 0)
            {
                auto touchZoomDistance = sqrt(a * a + b * b);
                if (_prevZoomTouchDistance && touchZoomDistance > 0)
                {
                    auto zoomLevel = touchZoomDistance / _prevZoomTouchDistance;
                    if (zoomLevel < 1)
                        zoomLevel = -(1 / zoomLevel);
                    zoomLevel *= 0.1;
                    _current_manipulator->touchZoomEvent(zoomLevel);
                }
                _prevZoomTouchDistance = touchZoomDistance;
            }
        }
        break;
    }
    }
    _previousTouches[touchMove.id] = &touchMove;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::dvec2 UpdateViewerHandler::ndc(const vsg::PointerEvent& event)
{
    auto renderArea = _camera->getRenderArea();
    auto [x, y] = cameraRenderAreaCoordinates(event);

    double aspectRatio = static_cast<double>(renderArea.extent.width) / static_cast<double>(renderArea.extent.height);
    vsg::dvec2 v(
        (renderArea.extent.width > 0) ? (static_cast<double>(x - renderArea.offset.x) / static_cast<double>(renderArea.extent.width) * 2.0 - 1.0) * aspectRatio : 0.0,
        (renderArea.extent.height > 0) ? static_cast<double>(y - renderArea.offset.y) / static_cast<double>(renderArea.extent.height) * 2.0 - 1.0 : 0.0);
    return v;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::pair<int32_t, int32_t> UpdateViewerHandler::cameraRenderAreaCoordinates(const vsg::PointerEvent& pointerEvent) const
{
    return {pointerEvent.x, pointerEvent.y};
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool UpdateViewerHandler::withinRenderArea(const vsg::PointerEvent& pointerEvent) const
{
    auto renderArea = _camera->getRenderArea();
    auto [x, y] = cameraRenderAreaCoordinates(pointerEvent);

    return (x >= renderArea.offset.x && x < static_cast<int32_t>(renderArea.offset.x + renderArea.extent.width)) &&
           (y >= renderArea.offset.y && y < static_cast<int32_t>(renderArea.offset.y + renderArea.extent.height));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool UpdateViewerHandler::isAlt()
{
    return (_keyboard->pressed(vsg::KEY_Alt_L) || _keyboard->pressed(vsg::KEY_Alt_R));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool UpdateViewerHandler::isCtrl()
{
    return (_keyboard->pressed(vsg::KEY_Control_L) || _keyboard->pressed(vsg::KEY_Control_R));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool UpdateViewerHandler::isShift()
{
    return (_keyboard->pressed(vsg::KEY_Shift_L) || _keyboard->pressed(vsg::KEY_Shift_R));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::changeCurrentVehicle()
{
    _current_manipulator->setCurrentVehicle(_vehicles_handler->getCurrentVehicle());

    _upd_server_control->changeCurrentVehicle(_vehicles_handler->getCurrentVehicleIndex(),
                                              _vehicles_handler->getControlledVehicleIndex());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::updateShadowRegion()
{
    // Текущий вид из камеры
    vsg::ref_ptr<vsg::LookAt> lookAt = _camera->viewMatrix.cast<vsg::LookAt>();

    // Еденичные векторы вперёд, вверх, вправо
    vsg::dvec3 orth = vsg::normalize(lookAt->center - lookAt->eye);
    vsg::dvec3 up = vsg::normalize(lookAt->up);
    vsg::dvec3 right = vsg::cross(orth, up);

    // Пересчитываем векторы, чтобы из них легко составить
    // квадрат, расположенный впереди на дистанции отрисовки теней
    orth = lookAt->eye +
           orth * _settings.shadow_distance;
    up = up * (_settings.shadow_distance / 2.0);
    right = right * (_settings.shadow_distance / 2.0);

    // Создаём пирамиду вида из камеры, в пределах которой будут рисоваться тени
    _shadow_region->points[0] = lookAt->eye;
    _shadow_region->points[1] = orth + up + right;
    _shadow_region->points[2] = orth + up - right;
    _shadow_region->points[3] = orth - up + right;
    _shadow_region->points[4] = orth - up - right;
}
