#include "UpdateViewerHandler.h"

#include <collision-route.h>

#include "UpdateControlToServerHandler.h"

#include "CameraCabineManipulator.h"
#include "CameraFollowManipulator.h"
#include "CameraFreeManipulator.h"
#include "CameraVehicleManipulator.h"
#include "CameraWalkManipulator.h"
#include "ScreenshotWriter.h"
#include "TrafficLightsHandler.h"
#include "MyGui.h"
#include "VehiclesHandler.h"
#include <vsg/utils/LineSegmentIntersector.h>
#include "settings.h"

#include <collision-world-loader.h>

#include <filesystem.h>
#include <Logger.h>

#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/ScrollWheelEvent.h>
#include <vsg/ui/TouchEvent.h>
#include <vsg/nodes/RegionOfInterest.h>

#ifdef _WIN32
    #include <windows.h>
#endif

#include <algorithm>
#include <cmath>

namespace
{

//------------------------------------------------------------------------------
/// Кватернион из базиса ПЕ (столбцы - образы локальных осей X/Y/Z).
/// Как в VehicleCollision::syncPose: тройка (orth, right, up) в RRS
/// левосторонняя, локальная ось Y тела - влево (-right)
//------------------------------------------------------------------------------
collision::Quatf quatFromBasis(const vsg::dvec3& o,
                               const vsg::dvec3& r,
                               const vsg::dvec3& u)
{
    const vsg::dvec3 left(-r.x, -r.y, -r.z);

    const double m00 = o.x, m01 = left.x, m02 = u.x;
    const double m10 = o.y, m11 = left.y, m12 = u.y;
    const double m20 = o.z, m21 = left.z, m22 = u.z;

    collision::Quatf q;

    const double trace = m00 + m11 + m22;
    if (trace > 0.0)
    {
        const double s = std::sqrt(trace + 1.0) * 2.0;
        q.w = static_cast<float>(0.25 * s);
        q.x = static_cast<float>((m21 - m12) / s);
        q.y = static_cast<float>((m02 - m20) / s);
        q.z = static_cast<float>((m10 - m01) / s);
    }
    else if (m00 > m11 && m00 > m22)
    {
        const double s = std::sqrt(1.0 + m00 - m11 - m22) * 2.0;
        q.w = static_cast<float>((m21 - m12) / s);
        q.x = static_cast<float>(0.25 * s);
        q.y = static_cast<float>((m01 + m10) / s);
        q.z = static_cast<float>((m02 + m20) / s);
    }
    else if (m11 > m22)
    {
        const double s = std::sqrt(1.0 + m11 - m00 - m22) * 2.0;
        q.w = static_cast<float>((m02 - m20) / s);
        q.x = static_cast<float>((m01 + m10) / s);
        q.y = static_cast<float>(0.25 * s);
        q.z = static_cast<float>((m12 + m21) / s);
    }
    else
    {
        const double s = std::sqrt(1.0 + m22 - m00 - m11) * 2.0;
        q.w = static_cast<float>((m10 - m01) / s);
        q.x = static_cast<float>((m02 + m20) / s);
        q.y = static_cast<float>((m12 + m21) / s);
        q.z = static_cast<float>(0.25 * s);
    }

    const float n = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    if (n > 1e-6f)
    {
        q.x /= n;
        q.y /= n;
        q.z /= n;
        q.w /= n;
    }

    return q;
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
UpdateViewerHandler::UpdateViewerHandler(
    vsg::ref_ptr<UpdateControlToServerHandler> upd_server_control,
    vsg::ref_ptr<vsg::Camera> camera,
    vsg::ref_ptr<vsg::Keyboard> keyboard,
    vsg::ref_ptr<vsg::RegionOfInterest> shadow_region,
    ScreenshotWriter* screenshot_writer,
    TrafficLightsHandler* sig_handler,
    VehiclesHandler* veh_handler,
    settings_t& settings,
    GUIParams* gui_params
)
    : Inherit()
    , gui_params(gui_params)
    , _settings(settings)
    , _keyboard(keyboard)
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

    // Пешая камера: физика игрока (параметры [Player] из settings.xml),
    // мир коллизий подгружается лениво при первом входе (F7)
    _walk_world = std::make_unique<collision::CollisionWorld>();
    _walk_manipulator = new CameraWalkManipulator(_keyboard, _camera, _settings,
                                                  _player, _walk_world.get());

    const FileSystem& fs = FileSystem::getInstance();
    const std::string settings_path =
            fs.getConfigDir() + fs.separator() + "settings.xml";
    _player.loadConfig(settings_path.c_str());

    _current_manipulator = _free_manipulator;
    _current_manipulator->resetView();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
UpdateViewerHandler::~UpdateViewerHandler() noexcept
{
    delete _walk_manipulator;
    delete _follow_manipulator;
    delete _cabine_manipulator;
    delete _vehicle_manipulator;
    delete _free_manipulator;

    if (_walk_world && _walk_world->isInitialized())
        _walk_world->shutdown();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::apply(vsg::FrameEvent& frame)
{
#ifdef _WIN32
    // VSG переводит сканкоды в VK через АКТИВНУЮ раскладку: если
    // пользователь переключится Alt+Shift, WASD и клавиши локомотива
    // снова превратятся в другие буквы. Держим английскую раскладку
    // потока постоянно (ввод по-русски в игре не нужен)
    static HKL en_layout = ::LoadKeyboardLayoutA("00000409", KLF_ACTIVATE);

    if ((en_layout != nullptr) && (::GetKeyboardLayout(0) != en_layout))
    {
        ::ActivateKeyboardLayout(en_layout, KLF_SETFORPROCESS);
    }
#endif

    if (frame.frameStamp->frameCount)
    {
        const double t = frame.frameStamp->simulationTime;
        const double dt = t - _previousTime;
        _previousTime = t;

        _sig_handler->step(static_cast<float>(t), static_cast<float>(dt));

        _vehicles_handler->step(t, dt);

        // Alt-подсказки и клики по органам кабины

        // Кинематические тела ПС в мире игрока - до шага физики игрока
        if (_current_manipulator == _walk_manipulator)
        {
            syncWalkVehicleBodies();
        }

        // Прицел на сиденье + автоподсказка "E - Сесть..." (ТЗ ходьба)
        if (_current_manipulator == _walk_manipulator)
        {
            updateSeatHint();
        }
        else if (gui_params != nullptr)
        {
            gui_params->walk_hint.clear();
        }

        _current_manipulator->frameEvent(dt);

        // Пол кабины следует за ПЕ + игрок "едет" вместе с ПЕ:
        // прикладываем дельту позиции ПЕ к игроку, иначе пол уезжает
        // из-под ног и бокс возвращается на едущего в нём игрока
        if (_inside_vehicle >= 0)
        {
            auto& vehicles = _vehicles_handler->vehicles;

            if (_inside_vehicle < static_cast<int>(vehicles.size()))
            {
                const vsg::dvec3 vpos =
                        vehicles[static_cast<size_t>(_inside_vehicle)].position;

                if (_inside_prev_vehicle_valid)
                {
                    const vsg::dvec3 delta = vpos - _inside_prev_vehicle_pos;

                    const double d = vsg::length(delta);

                    if ((d > 1.0e-6) && (d < 5.0))
                    {
                        _walk_manipulator->shiftPlayer(delta);
                    }
                }

                _inside_prev_vehicle_pos = vpos;
                _inside_prev_vehicle_valid = true;
            }

            updateInteriorFloor();
        }
        else
        {
            _inside_prev_vehicle_valid = false;
        }

        // Подъём по ступеням завершён - игрок стоит внутри кабины
        // (ходит по полу), камера остаётся пеший режим
        if (_boarding_pending && !_walk_manipulator->isBoarding())
        {
            _boarding_pending = false;

            if (auto look_at = _camera->viewMatrix.cast<vsg::LookAt>())
            {
                (void)look_at;
            }

            _walk_manipulator->teleportPlayer(_inside_stand_point);

            _inside_vehicle = _boarding_vehicle;
            _inside_cab = _boarding_cab;
        }

        // Игрок сошёл с пола кабины и отошёл от локомотива - вернуть
        // бокс кузова (раньше возвращали сразу: физика выбрасывала
        // игрока из кузова, отсюда "прыгание")
        if (_inside_vehicle >= 0 && !_boarding_pending)
        {
            const collision::Vec3f& pp =
                    _walk_manipulator->getPlayerPosition();

            auto& vehicles = _vehicles_handler->vehicles;

            if (_inside_vehicle < static_cast<int>(vehicles.size()))
            {
                const VehicleExterior& veh =
                        vehicles[static_cast<size_t>(_inside_vehicle)];

                const double dx = pp.x - veh.position.x;
                const double dy = pp.y - veh.position.y;

                // Габаритный бокс ПЕ - 30 м (полудлина 15): возвращаем
                // его только когда игрок ЗА габаритом, иначе бокс
                // материализуется вокруг игрока и "замуровывает" его
                if (std::sqrt(dx * dx + dy * dy) > 18.0)
                {
                    exitInterior();
                }
            }
            else
            {
                exitInterior();
            }
        }

        updateShadowRegion();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::setPause()
{
    int current_sf = _vehicles_handler->getSpeedFactor();

    if (current_sf == 0)
    {
        _upd_server_control->setSpeedFactor(1);
    }
    else
    {
        _upd_server_control->setSpeedFactor(0);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::apply(vsg::KeyPressEvent& keyPress)
{
    if (_keyboard)
    {
        keyPress.accept(*_keyboard);
    }

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

    if (keyPress.keyBase == vsg::KEY_Pause)
    {
        setPause();
        return;
    }

    // Enter - контекстно: в пешем режиме - вход в кабину ближайшей
    // ПЕ (если рядом есть дверь, порог тот же, что у Ctrl+Enter);
    // двери нет - выход из walk и взятие управления. Из свободной
    // камеры - сразу взятие управления
    if (!isCtrl() && !isShift() &&
        (keyPress.keyBase == vsg::KEY_Return ||
         keyPress.keyBase == vsg::KEY_KP_Enter) &&
        (_current_manipulator == _walk_manipulator ||
         _current_manipulator == _free_manipulator) &&
        _vehicles_handler->isUpdated())
    {
        if (_current_manipulator == _walk_manipulator)
        {
            // Уже внутри локомотива: Enter садит за пульт (камера
            // кабины), управление - как обычно Enter ещё раз
            if (_inside_vehicle >= 0)
            {
                exitInterior();
                enterCabineAt(_inside_vehicle, _inside_cab);
                return;
            }

            int vehicle_idx = -1;
            int cab_idx = -1;

            if (findNearestCabineDoor(vehicle_idx, cab_idx))
            {
                // Реалистичный подъём по ступеням, затем ходьба внутри
                startBoardingAnimation(vehicle_idx, cab_idx);
                return;
            }

            // Рядом нет двери: выходим из walk и берём управление
        }

        exitWalkMode();
        _vehicles_handler->selectControlVehicle();
        changeCurrentVehicle();
        _current_manipulator->resetView();

        return;
    }

    // Запрещаем управление выбором текущего вагона со свободной и
    // пешей камеры (в пешем режиме ПС выбирается клавишей E у кабины)
    if (_current_manipulator != _free_manipulator &&
        _current_manipulator != _walk_manipulator)
    {
        switch (keyPress.keyBase)
        {
            // Home - первый вагон следующего поезда на сервере
            case vsg::KEY_Home:
            {
                if (_vehicles_handler->selectNextTrain())
                {
                    changeCurrentVehicle();
                }

                return;
            }

            // End - первый вагон предыдущего поезда на сервере
            case vsg::KEY_End:
            {
                if (_vehicles_handler->selectPrevTrain())
                {
                    changeCurrentVehicle();
                }

                return;
            }

            // Page Up - следующий вагон поезда
            case vsg::KEY_Page_Up:
            {
                if (_vehicles_handler->selectNextVehicle())
                {
                    changeCurrentVehicle();
                }

                return;
            }

            // Page Down - предыдущий вагон поезда
            case vsg::KEY_Page_Down:
            {
                if (_vehicles_handler->selectPrevVehicle())
                {
                    changeCurrentVehicle();
                }

                return;
            }

            // Tab - сменить кабину
            case vsg::KEY_Tab:
            {
                changeCurrentCabine();
                _current_manipulator->resetView();
                return;
            }

            // Enter - взять управление текущим вагоном и кабиной
            // (Ctrl+Enter занят входом/выходом из локомотива - ниже)
            case vsg::KEY_KP_Enter:
            case vsg::KEY_Return:
            {
                if (isCtrl())
                {
                    break;
                }

                _vehicles_handler->selectControlVehicle();

                _upd_server_control->changeCurrentVehicle(
                            _vehicles_handler->getCurrentVehicleIndex(),
                            _vehicles_handler->getControlledVehicleIndex(),
                            _vehicles_handler->getCurrentVehicle()->controlled_cabine_idx);
                return;
            }

            default:
            {
                break;
            }
        }
    }

    // Во время подъёма по ступеням ввод ходьбы игнорируется
    if (_boarding_pending)
    {
        return;
    }

    // E - сесть на сиденье / встать (ТЗ ходьба): в пешем режиме при
    // наведении на сиденье машиниста (или помощника); сидя - встать
    // Win32 присылает строчные KeySym - принимаем оба регистра
    if (!isCtrl() && !isShift() &&
        (keyPress.keyBase == vsg::KEY_e || keyPress.keyBase == vsg::KEY_E) &&
        _current_manipulator == _walk_manipulator)
    {
        if (_walk_manipulator->isSeated())
        {
            // Встать: к двери кабины, где сидим
            auto& vehicles = _vehicles_handler->vehicles;

            if (seated_vehicle >= 0 &&
                static_cast<size_t>(seated_vehicle) < vehicles.size())
            {
                const VehicleExterior& veh =
                        vehicles[static_cast<size_t>(seated_vehicle)];

                size_t cab = std::min(seated_cab,
                                      veh.exit_pos.size() - 1);

                _walk_manipulator->standUp(
                            veh.worldFromLocal(veh.exit_pos[cab]));
            }

            if (gui_params != nullptr)
                gui_params->walk_hint.clear();

            return;
        }

        if (hint_vehicle >= 0)
        {
            auto& vehicles = _vehicles_handler->vehicles;

            if (static_cast<size_t>(hint_vehicle) < vehicles.size())
            {
                const VehicleExterior& veh =
                        vehicles[static_cast<size_t>(hint_vehicle)];

                if (hint_driver)
                {
                    // Место машиниста = рабочее место: кабина с
                    // управлением (как вход через дверь)
                    _vehicles_handler->selectVehicle(hint_vehicle);

                    VehicleExterior* v =
                            _vehicles_handler->getCurrentVehicle();

                    if (v != nullptr)
                        v->current_cabine_idx = hint_cab;

                    // Мы "внутри" этой ПЕ: F7 встанет пешком в кабину
                    _inside_vehicle = hint_vehicle;
                    _inside_cab = hint_cab;

                    _prev_manipulator = nullptr;
                    _upd_server_control->setControlSuppressed(false);
                    _current_manipulator = _cabine_manipulator;
                    _current_manipulator->setCurrentVehicle(v);
                    _current_manipulator->resetView();
                }
                else
                {
                    // Место помощника: фиксированная камера на сиденье,
                    // управление НЕ берётся
                    seated_vehicle = hint_vehicle;
                    seated_cab = hint_cab;
                    seated_driver = false;

                    if (hint_cab < veh.assistant_pos.size())
                        _walk_manipulator->sitDown(
                                    veh.worldFromLocal(
                                        veh.assistant_pos[hint_cab]),
                                    1.15);
                }
            }

            if (gui_params != nullptr)
                gui_params->walk_hint.clear();

            return;
        }
    }

    // Ctrl+Enter - вход/выход из локомотива (ТЗ "walking"):
    // из кабины - выход через дверь в пешей режим (спавн у ExitPos);
    // в пешем режиме - вход в кабину ПЕ, на чью дверь наведён взгляд
    if (isCtrl() && !isShift() &&
        (keyPress.keyBase == vsg::KEY_Return ||
         keyPress.keyBase == vsg::KEY_KP_Enter))
    {
        if (_current_manipulator == _walk_manipulator)
        {
            int vehicle_idx = -1;
            int cab_idx = -1;

            if (findNearestCabineDoor(vehicle_idx, cab_idx))
            {
                startBoardingAnimation(vehicle_idx, cab_idx);
            }
        }
        else if (_current_manipulator == _cabine_manipulator)
        {
            enterWalkMode();
        }

        return;
    }

    // Внешняя и свободная камеры перенесены на Shift+F3/Shift+F4:
    // F3/F4 без модификаторов заняты окном диагностики составов
    // (ТЗ "Промт статистики вагонов")
    if (isShift() && !isCtrl())
    {
        switch (keyPress.keyBase)
        {
            // Shift+F3 - внешняя камера текущей ПЕ
            case vsg::KEY_F3:
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

            // Shift+F4 - свободная камера
            case vsg::KEY_F4:
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

            default:
            {
                break;
            }
        }
    }

    // Управление F-клавишами только без Shift и Ctrl
    if (!isCtrl() && !isShift())
    {
        switch (keyPress.keyBase)
        {
            // F7 - пешая камера: включить пешей режим (1-е лицо) /
            // вернуть прежнюю камеру. Вход/выход из локомотива -
            // Ctrl+Enter (из кабины - к двери; в пешем - наведением
            // на дверь). 3-е лицо доступно только с места машиниста
            // (F1/Shift+F3/F5/F6)
            case vsg::KEY_F7:
            {
                if (_current_manipulator == _walk_manipulator)
                {
                    // Внутри локомотива F7 не выбрасывает: выход -
                    // спрыгнуть у двери или Ctrl+Enter из кабины
                    if (_inside_vehicle >= 0)
                    {
                        context_hint_walk();
                        return;
                    }

                    exitWalkMode();
                }
                else if (_current_manipulator == _cabine_manipulator &&
                         _inside_vehicle >= 0)
                {
                    // Встать с кресла внутрь кабины (ходьба по ПЕ)
                    standUpInsideCabine();
                }
                else
                {
                    enterWalkMode();
                }
                return;
            }

            // F1 - камера из кабины управляемой ПЕ
            case vsg::KEY_F1:
            {
                if (_vehicles_handler->returnToControlledVehicle() || (_current_manipulator != _cabine_manipulator))
                {
                    _current_manipulator = _cabine_manipulator;
                    changeCurrentVehicle();
                    _current_manipulator->resetView();
                    return;
                }

                _current_manipulator->returnView();
                return;
            }

            // F2 - камера из кабины текущей ПЕ
            case vsg::KEY_F2:
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

            // F5 - следящая камера справа по ходу поезда
            case vsg::KEY_F5:
            {
                _current_manipulator = _follow_manipulator;
                _current_manipulator->setCurrentVehicle(_vehicles_handler->getCurrentVehicle());
                _current_manipulator->resetView(); // Камера справа инициализируется в resetView
                return;
            }

            // F6 - следящая камера слева по ходу поезда
            case vsg::KEY_F6:
            {
                _current_manipulator = _follow_manipulator;
                _current_manipulator->setCurrentVehicle(_vehicles_handler->getCurrentVehicle());
                _current_manipulator->returnView(); // Камера слева инициализируется в returnView
                return;
            }

            // F12 - скриншот
            case vsg::KEY_F12:
            {
                _screenshot_writer->setScreenshot();
                return;
            }

            default:
            {
                break;
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    if (_keyboard)
    {
        keyRelease.accept(*_keyboard);
    }

    _current_manipulator->keyboardPressEvent(keyRelease.keyBase, false);   
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::apply(vsg::FocusInEvent& focusIn)
{
    if (_keyboard)
    {
        focusIn.accept(*_keyboard);
    }

#ifdef _WIN32
    // VSG переводит сканкоды в VK через АКТИВНУЮ раскладку
    // (MapVirtualKeyEx), поэтому в русской раскладке WASD и клавиши
    // локомотива приходят как другие буквы. Игра управляется
    // латинскими клавишами - включаем английскую раскладку для окна.
    static HKL en_layout = ::LoadKeyboardLayoutA("00000409", KLF_ACTIVATE);

    if (en_layout != nullptr)
    {
        ::ActivateKeyboardLayout(en_layout, KLF_SETFORPROCESS);
    }
#endif
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::apply(vsg::FocusOutEvent& focusOut)
{
    if (_keyboard)
    {
        focusOut.accept(*_keyboard);
    }

    // Потеря фокуса окна: отпускания клавиш не придёт - чистим
    // собственный набор пешей камеры, чтобы не залипали WASD
    if (_walk_manipulator != nullptr)
    {
        _walk_manipulator->clearKeys();
    }
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

    if (_hasPointerFocus)
    {
        buttonPress.handled = true;
    }

    _current_manipulator->mouseButtonPressEvent(buttonPress.button, buttonPress.mask, true);

    _previousPointerEvent = &buttonPress;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    if (buttonRelease.handled)
    {
        return;
    }

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

    // Пешая камера: обзор мышью при зажатой ПКМ (как в редакторе).
    // Если GUI (ImGui) захватил мышь - moveEvent.handled, обзор не крутим
    const bool walk_free_look = (_current_manipulator == _walk_manipulator) &&
                                _lastPointerEventWithinRenderArea &&
                                ((moveEvent.mask & vsg::BUTTON_MASK_3) != 0);

    if (moveEvent.handled || (!_hasPointerFocus && !walk_free_look))
    {
        return;
    }

    if (_previousPointerEvent)
    {
        const vsg::dvec2 new_ndc = ndc(moveEvent);
        const vsg::dvec2 prev_ndc = ndc(*_previousPointerEvent);
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
    if (scrollWheel.handled || !_lastPointerEventWithinRenderArea)
    {
        return;
    }

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
            const vsg::ref_ptr<vsg::Window> window = touchDown.window;
            const vsg::ref_ptr<vsg::ButtonPressEvent> event = vsg::ButtonPressEvent::create(
                window,
                touchDown.time,
                touchDown.x,
                touchDown.y,
                _current_manipulator->getTouchToButtonMask(),
                touchDown.id
            );
            apply(*event);
        }
        return;
    }
    case 1: {
        _prevZoomTouchDistance = 0.0;
        if (touchDown.id == 0 && _previousTouches.count(1))
        {
            const auto& prevTouch1 = _previousTouches[1];
            const double x = std::abs(static_cast<double>(prevTouch1->x) - touchDown.x);
            const double y = std::abs(static_cast<double>(prevTouch1->y) - touchDown.y);
            if (x > 0 || y > 0)
            {
                _prevZoomTouchDistance = std::sqrt(x * x + y * y);
            }
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
        const vsg::ref_ptr<vsg::Window> window = touchUp.window;
        const vsg::ref_ptr<vsg::ButtonReleaseEvent> event = vsg::ButtonReleaseEvent::create(
            window,
            touchUp.time,
            touchUp.x,
            touchUp.y,
            _current_manipulator->getTouchToButtonMask(),
            touchUp.id
        );
        apply(*event);
    }
    _previousTouches.erase(touchUp.id);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::apply(vsg::TouchMoveEvent& touchMove)
{
    vsg::ref_ptr<vsg::Window> window = touchMove.window;
    switch (_previousTouches.size())
    {
    case 1: {
        // Rotate
        const vsg::ref_ptr<vsg::MoveEvent> event = vsg::MoveEvent::create(
            window,
            touchMove.time,
            touchMove.x,
            touchMove.y,
            _current_manipulator->getTouchToButtonMask()
        );
        apply(*event);
        break;
    }
    case 2: {
        if (touchMove.id == 0 && _previousTouches.count(0))
        {
            // Zoom
            const auto& prevTouch1 = _previousTouches[1];
            const double x = std::abs(static_cast<double>(prevTouch1->x) - touchMove.x);
            const double y = std::abs(static_cast<double>(prevTouch1->y) - touchMove.y);
            if (x > 0 || y > 0)
            {
                const double touchZoomDistance = std::sqrt(x * x + y * y);
                if (_prevZoomTouchDistance && touchZoomDistance > 0)
                {
                    double zoomLevel = touchZoomDistance / _prevZoomTouchDistance;
                    if (zoomLevel < 1.0)
                    {
                        zoomLevel = -(1.0 / zoomLevel);
                    }
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
vsg::dvec2 UpdateViewerHandler::ndc(const vsg::PointerEvent& event) const
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
bool UpdateViewerHandler::isAlt() const
{
    return (_keyboard->pressed(vsg::KEY_Alt_L) || _keyboard->pressed(vsg::KEY_Alt_R));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool UpdateViewerHandler::isCtrl() const
{
    return (_keyboard->pressed(vsg::KEY_Control_L) || _keyboard->pressed(vsg::KEY_Control_R));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool UpdateViewerHandler::isShift() const
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
                                              _vehicles_handler->getControlledVehicleIndex(),
                                              _vehicles_handler->getCurrentVehicle()->controlled_cabine_idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::changeCurrentCabine()
{
    VehicleExterior* vehicle = _vehicles_handler->getCurrentVehicle();

    size_t cabs_num = vehicle->driver_pos.size();

    if (cabs_num < 2)
    {
        vehicle->current_cabine_idx = 0;
        return;
    }

    vehicle->current_cabine_idx++;

    if (vehicle->current_cabine_idx == cabs_num)
    {
        vehicle->current_cabine_idx = 0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::initWalkWorld()
{
    if (_walk_world_loaded)
        return;

    _walk_world_loaded = true;

    collision::WorldSettings world_settings;
    // Пешему миру не нужны события контактов и большие лимиты: только
    // лучи (земля/стены/ступеньки) и кинематические боксы ПС
    world_settings.max_bodies = 8192;
    world_settings.max_body_pairs = 4096;
    world_settings.max_contact_constraints = 2048;
    world_settings.temp_allocator_size = 4 * 1024 * 1024;
    world_settings.worker_threads = 1;

    if (!_walk_world->init(world_settings))
    {
        LOG_WARN("Walk mode: collision world init failed");
        return;
    }

    // Статическая геометрия маршрута (objects.ref + route1.map +
    // colliders.conf). Без неё игрок ходит по бесконечному полу z = 0
    collision::WorldLoadStats stats;
    std::string error;

    const std::string route_dir = _settings.route_dir_full_path;

    if (!collision::loadRouteIntoWorld(*_walk_world, route_dir, stats, &error))
    {
        LOG_WARN("Walk mode: route collision geometry not loaded "
                 "(player walks on flat ground): %s", error.c_str());
    }
    else
    {
        LOG_INFO("Walk mode: loaded %u collision bodies from route",
                 static_cast<unsigned int>(stats.objects_created));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::syncWalkVehicleBodies()
{
    if (!_walk_world->isInitialized())
        return;

    auto& vehicles = _vehicles_handler->vehicles;
    const size_t count = vehicles.size();

    // Новые ПС появились на сервере - добавляем тела
    while (_walk_vehicle_bodies.size() < count)
    {
        // Габаритный бокс ПС: длина до 15 м (полудлина), ширина ~3.2 м,
        // высота 4 м от уровня головки рельса (позиция ПЕ - на уровне
        // рельса в центре кузова)
        collision::ObjectDesc desc;
        desc.shape = collision::CollisionShape::box(
                    collision::Vec3f(15.0f, 1.6f, 2.0f));
        desc.layer = collision::Layer::Train;
        desc.motion = collision::MotionType::Kinematic;
        desc.position = collision::Vec3f(0.0f, 0.0f, -100.0f);

        _walk_vehicle_bodies.push_back(_walk_world->createObject(desc));
    }

    for (size_t i = 0; i < count; ++i)
    {
        collision::CollisionObject& object = _walk_vehicle_bodies[i];

        if (!object.isValid())
            continue;

        // Игрок внутри этой ПЕ: бокс кузова УБИРАЕТСЯ из мира
        // (иначе коллизии выталкивают игрока, бесконечные прыжки)
        if (static_cast<int>(i) == _inside_vehicle)
        {
            object.setPositionRotation(
                        collision::Vec3f(0.0f, -10000.0f, -10000.0f),
                        collision::quatFromRouteEuler(
                            collision::Vec3f(0.0f, 0.0f, 0.0f)));
            continue;
        }

        const VehicleExterior& vehicle = vehicles[i];

        const vsg::dvec3 center = vehicle.position +
                vehicle.up * 2.0;

        const collision::Vec3f pos(
                    static_cast<float>(center.x),
                    static_cast<float>(center.y),
                    static_cast<float>(center.z));

        object.setPositionRotation(
                    pos,
                    quatFromBasis(vehicle.orth,
                                  vehicle.right,
                                  vehicle.up));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::enterWalkMode()
{
    initWalkWorld();

    _prev_manipulator = _current_manipulator;

    // Клавиши управляют игроком, а не поездом (в т.ч. зажатые до входа)
    _upd_server_control->setControlSuppressed(true);

    // Спавн: из кабины - у точки выхода текущей кабины; иначе - рядом
    // с текущей ПЕ (или у свободной камеры, если ПС ещё нет)
    vsg::dvec3 spawn = _lookAt_of_camera_eye();

    VehicleExterior* vehicle = _vehicles_handler->getCurrentVehicle();

    if (vehicle && (_current_manipulator == _cabine_manipulator) &&
        !vehicle->exit_pos.empty())
    {
        const size_t cab = std::min(vehicle->current_cabine_idx,
                                    vehicle->exit_pos.size() - 1);

        const vsg::dvec3 door_w = vehicle->worldFromLocal(
                    vehicle->exit_pos[cab]);

        // Спавн чуть наружу от кузова: внутри бокса кузова физика
        // выбрасывает игрока
        vsg::dvec3 seat_w = vehicle->worldFromLocal(
                    vehicle->driver_pos[std::min(cab,
                        vehicle->driver_pos.size() - 1)]);

        vsg::dvec3 inward_w = seat_w - door_w;
        inward_w.z = 0.0;
        const double inward_len = vsg::length(inward_w);
        inward_w = (inward_len > 1.0e-6) ?
                    (inward_w / inward_len) : vsg::dvec3(0.0, 0.0, 0.0);

        spawn = door_w - inward_w * 1.2;
    }
    else if (vehicle)
    {
        spawn = vehicle->position + vehicle->right * 3.0 +
                vehicle->up * 1.0;
    }

    _current_manipulator = _walk_manipulator;
    _walk_manipulator->clearKeys();
    _walk_manipulator->activate(spawn);
    _walk_manipulator->resetView();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::exitWalkMode()
{
    if (_current_manipulator != _walk_manipulator)
        return;

    if (_inside_vehicle >= 0)
    {
        exitInterior();
    }

    // Возврат к камере, из которой вошли (кабина/свободная)
    CameraAbstract* target = _prev_manipulator;

    if (target == nullptr)
        target = _free_manipulator;

    // Управление поездом снова на клавишах
    _upd_server_control->setControlSuppressed(false);

    _current_manipulator = target;
    _prev_manipulator = nullptr;

    _current_manipulator->setCurrentVehicle(_vehicles_handler->getCurrentVehicle());
    _current_manipulator->resetView();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::updateSeatHint()
{
    hint_vehicle = -1;

    if (gui_params != nullptr)
        gui_params->walk_hint.clear();

    if (_walk_manipulator->isSeated())
    {
        // Сидим: подсказку не показываем (встать - просто E)
        return;
    }

    auto& vehicles = _vehicles_handler->vehicles;

    if (vehicles.empty())
        return;

    vsg::ref_ptr<vsg::LookAt> lookAt =
            _camera->viewMatrix.cast<vsg::LookAt>();

    if (!lookAt)
        return;

    const vsg::dvec3 eye = lookAt->eye;
    const vsg::dvec3 aim =
            vsg::normalize(lookAt->center - lookAt->eye);

    // Сиденье в конусе взгляда (~20 град) и ближе 3 м
    const double max_cos = std::cos(vsg::radians(35.0));
    const double max_dist = 3.0;

    double best_cos = max_cos;

    for (size_t i = 0; i < vehicles.size(); ++i)
    {
        const VehicleExterior& vehicle = vehicles[i];

        for (size_t cab = 0; cab < vehicle.driver_pos.size(); ++cab)
        {
            const vsg::dvec3 seat =
                    vehicle.worldFromLocal(vehicle.driver_pos[cab]);
            const vsg::dvec3 to_seat = seat - eye;
            const double dist = vsg::length(to_seat);

            if (dist > max_dist || dist < 0.3)
                continue;

            const double cos_a = vsg::dot(aim, to_seat / dist);

            if (cos_a > best_cos)
            {
                best_cos = cos_a;
                hint_vehicle = static_cast<int>(i);
                hint_cab = cab;
                hint_driver = true;
            }
        }

        for (size_t cab = 0; cab < vehicle.assistant_pos.size(); ++cab)
        {
            const vsg::dvec3 seat =
                    vehicle.worldFromLocal(vehicle.assistant_pos[cab]);
            const vsg::dvec3 to_seat = seat - eye;
            const double dist = vsg::length(to_seat);

            if (dist > max_dist || dist < 0.3)
                continue;

            const double cos_a = vsg::dot(aim, to_seat / dist);

            if (cos_a > best_cos)
            {
                best_cos = cos_a;
                hint_vehicle = static_cast<int>(i);
                hint_cab = cab;
                hint_driver = false;
            }
        }
    }

    if (hint_vehicle >= 0 && gui_params != nullptr)
    {
        gui_params->walk_hint = (hint_driver) ?
                QString("E — Сесть на место машиниста") :
                QString("E — Сесть на место помощника");
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::tryEnterNearestCabine()
{
    int vehicle_idx = -1;
    int cab_idx = -1;

    if (findNearestCabineDoor(vehicle_idx, cab_idx))
    {
        enterCabineAt(vehicle_idx, cab_idx);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool UpdateViewerHandler::findNearestCabineDoor(int& vehicle_idx, int& cab_idx)
{
    auto& vehicles = _vehicles_handler->vehicles;

    if (vehicles.empty())
        return false;

    // Наведение на дверь: направление взгляда игрока (камера 1-го лица)
    vsg::ref_ptr<vsg::LookAt> lookAt = _camera->viewMatrix.cast<vsg::LookAt>();

    if (!lookAt)
        return false;

    const vsg::dvec3 eye = lookAt->eye;
    const vsg::dvec3 aim = vsg::normalize(lookAt->center - lookAt->eye);

    // Дверь должна быть в конусе взгляда (~40 град) и рядом
    const double max_cos = std::cos(vsg::radians(40.0));
    const double max_dist = _settings.walk_interact_distance;

    int best_vehicle = -1;
    size_t best_cab = 0;
    double best_cos = max_cos;

    // Fallback: если конус взгляда не поймал дверь, берём ближайшую
    // в радиусе взаимодействия (без ограничения по углу) - чтобы
    // не требовать от игрока точного прицеливания
    int near_vehicle = -1;
    size_t near_cab = 0;
    double near_dist = max_dist;

    for (size_t i = 0; i < vehicles.size(); ++i)
    {
        const VehicleExterior& vehicle = vehicles[i];

        for (size_t cab = 0; cab < vehicle.exit_pos.size(); ++cab)
        {
            // Мировая позиция двери кабины (ExitPos; дверь - она же
            // точка выхода/входа игрока)
            const vsg::dvec3 door =
                    vehicle.worldFromLocal(vehicle.exit_pos[cab]);

            const vsg::dvec3 to_door = door - eye;

            const double dist = vsg::length(to_door);

            if (dist > max_dist || dist < 0.2)
                continue;

            if (dist < near_dist)
            {
                near_dist = dist;
                near_vehicle = static_cast<int>(i);
                near_cab = cab;
            }

            const double cos_angle =
                    vsg::dot(aim, to_door / dist);

            if (cos_angle > best_cos)
            {
                best_cos = cos_angle;
                best_vehicle = static_cast<int>(i);
                best_cab = cab;
            }
        }
    }

    if (best_vehicle < 0)
    {
        best_vehicle = near_vehicle;
        best_cab = near_cab;
    }

    if (best_vehicle < 0)
        return false;

    vehicle_idx = best_vehicle;
    cab_idx = best_cab;
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::enterCabineAt(int vehicle_idx, int cab_idx)
{
    auto& vehicles = _vehicles_handler->vehicles;

    if (vehicle_idx < 0 ||
        vehicle_idx >= static_cast<int>(vehicles.size()))
    {
        return;
    }

    _vehicles_handler->selectVehicle(vehicle_idx);

    VehicleExterior* vehicle = _vehicles_handler->getCurrentVehicle();

    if (vehicle != nullptr)
        vehicle->current_cabine_idx = cab_idx;

    // Выход из пешего режима в кабину: клавиши снова управляют поездом
    exitInterior();
    _prev_manipulator = nullptr;
    _upd_server_control->setControlSuppressed(false);
    _current_manipulator = _cabine_manipulator;
    _current_manipulator->setCurrentVehicle(vehicle);
    _current_manipulator->resetView();
}

//------------------------------------------------------------------------------
// Подъём по ступеням: путь глаза игрока -> подход -> ступени -> порог
// -> внутрь -> место машиниста; анимирует пешая камера, по завершении
// FrameEvent переключает камеру в кабину
//------------------------------------------------------------------------------
void UpdateViewerHandler::startBoardingAnimation(int vehicle_idx, int cab_idx)
{
    auto& vehicles = _vehicles_handler->vehicles;

    if (vehicle_idx < 0 ||
        vehicle_idx >= static_cast<int>(vehicles.size()))
    {
        return;
    }

    const VehicleExterior& vehicle = vehicles[static_cast<size_t>(vehicle_idx)];

    if (cab_idx >= static_cast<int>(vehicle.exit_pos.size()) ||
        cab_idx >= static_cast<int>(vehicle.driver_pos.size()))
    {
        return;
    }

    vsg::ref_ptr<vsg::LookAt> lookAt = _camera->viewMatrix.cast<vsg::LookAt>();

    if (!lookAt)
        return;

    const vsg::dvec3 eye0 = lookAt->eye;

    const vsg::dvec3 door = vehicle.worldFromLocal(vehicle.exit_pos[cab_idx]);

    // Уровень земли у двери: глаза выше неё на высоту глаз игрока
    const double ground_z = eye0.z - _walk_manipulator->playerEyeHeight();

    // Горизонтальное направление "игрок -> дверь" (наружу)
    vsg::dvec3 out = door - eye0;
    out.z = 0.0;
    const double out_len = vsg::length(out);
    out = (out_len > 1.0e-6) ? (out / out_len) : vsg::dvec3(0.0, 0.0, 0.0);

    const vsg::dvec3 seat_local = vehicle.driver_pos[cab_idx];
    const vsg::dvec3 seat_world = vehicle.position +
            vehicle.right * seat_local.x +
            vehicle.orth * seat_local.y +
            vehicle.up * seat_local.z;

    // Горизонтальное направление "дверь -> внутрь"
    vsg::dvec3 inward = seat_world - door;
    inward.z = 0.0;
    const double inward_len = vsg::length(inward);
    inward = (inward_len > 1.0e-6) ? (inward / inward_len) : -out;

    std::vector<vsg::dvec3> path;
    path.reserve(6);

    // Пол кабины на уровне порога двери: стоим и ходим внутри
    const double floor_z = door.z;
    const double eye_h = _walk_manipulator->playerEyeHeight();

    const vsg::dvec3 stand_point = door + inward * 1.3 +
            vsg::dvec3(0.0, 0.0, 0.0);

    path.push_back(eye0);
    path.push_back(door + out * 0.9 +
                   vsg::dvec3(0.0, 0.0, ground_z - eye0.z + 0.15));
    path.push_back(door + out * 0.25 +
                   vsg::dvec3(0.0, 0.0, (floor_z - eye0.z) * 0.6));
    path.push_back(door + vsg::dvec3(0.0, 0.0, eye_h - ground_z));
    path.push_back(stand_point +
                   vsg::dvec3(0.0, 0.0, eye_h - ground_z));

    addInteriorFloor(vehicle_idx, inward, door);

    _inside_stand_point = stand_point +
            vsg::dvec3(0.0, 0.0, 0.45);
    _inside_floor_z = floor_z;

    _boarding_vehicle = vehicle_idx;
    _boarding_cab = cab_idx;
    _boarding_pending = true;

    _walk_manipulator->startBoarding(path, 1.6);
}

//------------------------------------------------------------------------------
// Статический бокс-пол кабины: верх на уровне порога двери, доска
// вдоль борта от двери вглубь кабины. Один бокс на ПЕ
//------------------------------------------------------------------------------
void UpdateViewerHandler::addInteriorFloor(int vehicle_idx,
                                           const vsg::dvec3& inward,
                                           const vsg::dvec3& door)
{
    if (vehicle_idx < 0)
    {
        return;
    }

    if (static_cast<int>(_interior_floor_added.size()) <= vehicle_idx)
    {
        _interior_floor_added.resize(vehicle_idx + 1, false);
        _interior_floor_bodies.resize(vehicle_idx + 1);
    }

    if (_interior_floor_added[vehicle_idx])
    {
        return;
    }

    auto& vehicles = _vehicles_handler->vehicles;

    if (vehicle_idx >= static_cast<int>(vehicles.size()))
    {
        return;
    }

    const VehicleExterior& vehicle = vehicles[static_cast<size_t>(vehicle_idx)];

    if (vehicle.exit_pos.empty())
    {
        return;
    }

    const double yaw_deg = std::atan2(vehicle.orth.x, vehicle.orth.y) *
            57.2957795;

    collision::ObjectDesc desc;
    desc.shape = collision::CollisionShape::box(
                collision::Vec3f(2.2f, 1.6f, 0.1f));
    desc.layer = collision::Layer::Infrastructure;
    desc.motion = collision::MotionType::Kinematic;
    desc.position = collision::Vec3f(
                static_cast<float>(door.x + inward.x * 1.2),
                static_cast<float>(door.y + inward.y * 1.2),
                static_cast<float>(door.z - 0.1));
    desc.rotation = collision::quatFromRouteEuler(
                collision::Vec3f(0.0f, 0.0f,
                                 static_cast<float>(-yaw_deg)));

    _interior_floor_bodies[vehicle_idx] =
            _walk_world->createObject(desc);

    _interior_floor_added[vehicle_idx] = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::dvec3 UpdateViewerHandler::_lookAt_of_camera_eye()
{
    vsg::ref_ptr<vsg::LookAt> lookAt = _camera->viewMatrix.cast<vsg::LookAt>();
    return lookAt ? lookAt->eye : vsg::dvec3(0.0, 0.0, 2.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::updateShadowRegion()
{
    // Текущий вид из камеры
    vsg::ref_ptr<vsg::LookAt> lookAt = _camera->viewMatrix.cast<vsg::LookAt>();

    // Единичные векторы вперёд, вверх, вправо
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::exitInterior()
{
    _inside_vehicle = -1;
    _inside_cab = -1;
}


//------------------------------------------------------------------------------
// F7 из кабины: встать с кресла пешком внутрь кабины (ходьба по ПЕ)
//------------------------------------------------------------------------------
void UpdateViewerHandler::standUpInsideCabine()
{
    auto& vehicles = _vehicles_handler->vehicles;
    VehicleExterior* vehicle = _vehicles_handler->getCurrentVehicle();

    if (vehicle == nullptr || vehicle->exit_pos.empty() ||
        vehicle->driver_pos.empty())
    {
        enterWalkMode();
        return;
    }

    const size_t cab = std::min(vehicle->current_cabine_idx,
                                vehicle->exit_pos.size() - 1);

    const vsg::dvec3 door = vehicle->worldFromLocal(vehicle->exit_pos[cab]);

    const vsg::dvec3 seat_world = vehicle->worldFromLocal(
                vehicle->driver_pos[std::min(cab,
                    vehicle->driver_pos.size() - 1)]);

    vsg::dvec3 inward = seat_world - door;
    inward.z = 0.0;
    const double inward_len = vsg::length(inward);
    inward = (inward_len > 1.0e-6) ?
                (inward / inward_len) : vsg::dvec3(0.0, 0.0, 0.0);

    addInteriorFloor(_inside_vehicle, inward, door);

    _inside_floor_z = door.z;
    _inside_stand_point = door + inward * 1.3 +
            vsg::dvec3(0.0, 0.0, 0.45);

    _prev_manipulator = nullptr;
    _upd_server_control->setControlSuppressed(true);
    _current_manipulator = _walk_manipulator;
    _current_manipulator->setCurrentVehicle(vehicle);

    _walk_manipulator->teleportPlayer(_inside_stand_point);
    _current_manipulator->resetView();

    if (gui_params != nullptr)
        gui_params->walk_hint.clear();
}

//------------------------------------------------------------------------------
// Подсказка вместо действия (F7 внутри локомотива)
//------------------------------------------------------------------------------
void UpdateViewerHandler::context_hint_walk()
{
    if (gui_params != nullptr)
    {
        gui_params->walk_hint =
                QString::fromUtf8(u8"Выйдите из локомотива: спрыгните у двери");
    }
}

//------------------------------------------------------------------------------
// Пол кабины следует за ПЕ (кинематика): пересчёт позиции плиты по
// текущей позе двери. Также обновляет точку стояния и высоту пола
//------------------------------------------------------------------------------
void UpdateViewerHandler::updateInteriorFloor()
{
    if (_inside_vehicle < 0)
        return;

    auto& vehicles = _vehicles_handler->vehicles;

    if (_inside_vehicle >= static_cast<int>(vehicles.size()))
        return;

    const VehicleExterior& vehicle = vehicles[static_cast<size_t>(_inside_vehicle)];

    if (vehicle.exit_pos.empty() || vehicle.driver_pos.empty())
        return;

    const size_t cab = std::min(static_cast<size_t>(_inside_cab),
                                vehicle.exit_pos.size() - 1);

    const vsg::dvec3 door = vehicle.worldFromLocal(vehicle.exit_pos[cab]);

    const vsg::dvec3 seat_world = vehicle.worldFromLocal(
                vehicle.driver_pos[std::min(cab,
                    vehicle.driver_pos.size() - 1)]);

    vsg::dvec3 inward = seat_world - door;
    inward.z = 0.0;
    const double inward_len = vsg::length(inward);
    inward = (inward_len > 1.0e-6) ?
                (inward / inward_len) : vsg::dvec3(0.0, 0.0, 0.0);

    const double yaw_deg = std::atan2(vehicle.orth.x, vehicle.orth.y) *
            57.2957795;

    if (static_cast<int>(_interior_floor_bodies.size()) > _inside_vehicle)
    {
        collision::CollisionObject& floor_object =
                _interior_floor_bodies[static_cast<size_t>(_inside_vehicle)];

        if (floor_object.isValid())
        {
            floor_object.setPositionRotation(
                        collision::Vec3f(
                            static_cast<float>(door.x + inward.x * 1.2),
                            static_cast<float>(door.y + inward.y * 1.2),
                            static_cast<float>(door.z - 0.1)),
                        collision::quatFromRouteEuler(
                            collision::Vec3f(0.0f, 0.0f,
                                             static_cast<float>(-yaw_deg))));
        }
    }

    _inside_floor_z = door.z;
    _inside_stand_point = door + inward * 1.3 +
            vsg::dvec3(0.0, 0.0, 0.45);
}
