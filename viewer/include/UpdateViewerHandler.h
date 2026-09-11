#pragma once
#ifndef UPDATE_VIEWER_HANDLER_H
#define UPDATE_VIEWER_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/Visitor.h>
#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>

#include <player-controller.h>
#include <collision-world.h>
#include <collision-object.h>


#include <cstdint>
#include <map>
#include <memory>
#include <utility>
#include <vector>

class CameraAbstract;
struct GUIParams;
class CameraWalkManipulator;
class ScreenshotWriter;
struct settings_t;
class TrafficLightsHandler;
class UpdateControlToServerHandler;
class VehiclesHandler;
class VehicleExterior;

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
        vsg::ref_ptr<vsg::Keyboard> keyboard,
        vsg::ref_ptr<vsg::RegionOfInterest> shadow_region,
        ScreenshotWriter* screenshot_writer,
        TrafficLightsHandler* sig_handler,
        VehiclesHandler* veh_handler,
        settings_t& settings,
        GUIParams* gui_params = nullptr
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

    void setKeyboard(vsg::ref_ptr<vsg::Keyboard> keyboard)
    {
        _keyboard = keyboard;
    }

private:
    /// compute non-dimensional window coordinate (-1, 1) from event coords
    vsg::dvec2 ndc(const vsg::PointerEvent& event) const;

    std::pair<std::int32_t, std::int32_t> cameraRenderAreaCoordinates(const vsg::PointerEvent& pointerEvent) const;

    bool withinRenderArea(const vsg::PointerEvent& pointerEvent) const;

    bool isAlt() const;
    bool isCtrl() const;
    bool isShift() const;

    void changeCurrentVehicle();

    void changeCurrentCabine();

    void updateShadowRegion();

    /// Пешая ходьба (ТЗ "walking"): вход/выход из пешего режима,
    /// вход в кабину ближайшей ПЕ по клавише E
    void enterWalkMode();
    void exitWalkMode();
    void tryEnterNearestCabine();

    /// Найти дверь ближайшей ПЕ (true - найдена)
    bool findNearestCabineDoor(int& vehicle_idx, int& cab_idx);

    /// Мгновенный вход в кабину (выбор ПЕ + камера кабины)
    void enterCabineAt(int vehicle_idx, int cab_idx);

    /// Построить путь подъёма по ступеням и запустить анимацию
    void startBoardingAnimation(int vehicle_idx, int cab_idx);

    /// Добавить статический пол кабины в мир коллизий (один раз)
    void addInteriorFloor(int vehicle_idx, const vsg::dvec3& inward,
                          const vsg::dvec3& door);

    /// Выход из состояния "внутри ПЕ" (вернуть бокс кузова)
    void exitInterior();

    /// Обновить позу пола кабины (следует за ПЕ)
    void updateInteriorFloor();

    /// F7 из кабины: встать пешком внутрь кабины
    void standUpInsideCabine();

    /// Подсказка (вместо действия внутри ПЕ)
    void context_hint_walk();

    /// Ленивая инициализация локального мира коллизий пешего режима:
    /// статические объекты маршрута (loadRouteIntoWorld)
    void initWalkWorld();

    /// Синхронизация кинематических тел ПС в мире игрока
    void syncWalkVehicleBodies();

    /// Прицел на сиденье (машинист/помощник) в пешем режиме:
    /// ищет сиденье в конусе взгляда, обновляет подсказку GUI
    void updateSeatHint();

    /// Контекст посадки (E): где сидим и куда встать
    int seated_vehicle = -1;
    size_t seated_cab = 0;
    bool seated_driver = false;

    /// Результат прицела: кандидаты на посадку по E
    int hint_vehicle = -1;
    size_t hint_cab = 0;
    bool hint_driver = true;
    GUIParams* gui_params = nullptr;

    /// Текущее положение глаза камеры (спавн пешего режима)
    vsg::dvec3 _lookAt_of_camera_eye();

    settings_t& _settings;
    vsg::ref_ptr<vsg::Keyboard> _keyboard;

    /// Анимация входа в кабину
    bool _boarding_pending = false;
    int _boarding_vehicle = -1;
    int _boarding_cab = -1;

    /// Игрок находится внутри локомотива (ходьба по полу кабины):
    /// индекс ПЕ, кабина, высота пола, точка выхода из анимации.
    /// Кинематический бокс кузова на это время скрыт
    int _inside_vehicle = -1;
    int _inside_cab = -1;
    double _inside_floor_z = 0.0;
    vsg::dvec3 _inside_stand_point = {0.0, 0.0, 0.0};
    vsg::dvec3 _inside_prev_vehicle_pos = {0.0, 0.0, 0.0};
    bool _inside_prev_vehicle_valid = false;
    std::vector<bool> _interior_floor_added;
    std::vector<collision::CollisionObject> _interior_floor_bodies;
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

    /// Пешая камера от 1-го лица (ТЗ "walking"). Третье лицо остаётся
    /// только у места машиниста (внешние/следящие камеры)
    CameraWalkManipulator* _walk_manipulator = nullptr;

    /// Камера, из которой вошли в пешей режим (возврат по F7)
    CameraAbstract* _prev_manipulator = nullptr;

    /// Физика пешего игрока и его локальный мир коллизий (маршрут
    /// + кинематические боксы ПС из интерполяции VehiclesHandler)
    PlayerController _player;
    std::unique_ptr<collision::CollisionWorld> _walk_world;
    bool _walk_world_loaded = false;
    std::vector<collision::CollisionObject> _walk_vehicle_bodies;

    ScreenshotWriter* _screenshot_writer = nullptr;
    TrafficLightsHandler* _sig_handler = nullptr;
    VehiclesHandler* _vehicles_handler = nullptr;

    //--- Alt-взаимодействие с органами кабины (ТЗ "Взаимодействие с
    // элементами кабины"): пикинг мешей органов в модели кабины,
    // подсказка и клик -> инжект штатной клавиши устройства ---

    /// Alt удерживается (режим подсказок кабины)

    /// Пикинг органа под курсором (Alt удерживается)

    /// Клик по органу: primary - ЛКМ (включить/вперёд), иначе ПКМ

    /// Подсказка Alt: обновление состояния (каждый кадр, пикинг ~20 Гц)


    bool _wasPausePhysicallyPressed = false;
    void setPause();
};

#endif // UPDATE_VIEWER_HANDLER_H
