h = open(r"viewer\include\UpdateViewerHandler.h", encoding="utf-8").read()

a = "class CameraAbstract;"
assert a in h
h = h.replace(a, a + "\nstruct GUIParams;", 1)

b = """    /// Синхронизация кинематических тел ПС в мире игрока
    void syncWalkVehicleBodies();"""
assert b in h
h = h.replace(b, b + """

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
    GUIParams* gui_params = nullptr;""", 1)
open(r"viewer\include\UpdateViewerHandler.h", "w", encoding="utf-8").write(h)

c = open(r"viewer\src\UpdateViewerHandler.cpp", encoding="utf-8").read()

# ctor: + gui_params (после settings)
old_ctor = """        ScreenshotWriter* screenshot_writer,
        TrafficLightsHandler* sig_handler,
        VehiclesHandler* veh_handler,
        settings_t& settings
    )
        : Inherit()"""
assert old_ctor in c
new_ctor = """        ScreenshotWriter* screenshot_writer,
        TrafficLightsHandler* sig_handler,
        VehiclesHandler* veh_handler,
        settings_t& settings,
        GUIParams* gui_params_ = nullptr
    )
        : Inherit()
        , gui_params(gui_params_)"""
c = c.replace(old_ctor, new_ctor, 1)

# FrameEvent: обновление подсказки в пешем режиме
old_frame = """        // Кинематические тела ПС в мире игрока - до шага физики игрока
        if (_current_manipulator == _walk_manipulator)
        {
            syncWalkVehicleBodies();
        }"""
assert old_frame in c
c = c.replace(old_frame, old_frame + """

        // Прицел на сиденье + автоподсказка "E - Сесть..." (ТЗ ходьба)
        if (_current_manipulator == _walk_manipulator)
        {
            updateSeatHint();
        }
        else if (gui_params != nullptr)
        {
            gui_params->walk_hint.clear();
        }""", 1)

# E: обработка в keyPress — вставка перед Shift+F3 блоком
old_e = """    // Ctrl+Enter - вход/выход из локомотива (ТЗ "walking"):"""
assert old_e in c
new_e = """    // E - сесть на сиденье / встать (ТЗ ходьба): в пешем режиме при
    // наведении на сиденье машиниста (или помощника); сидя - встать
    if (!isCtrl() && !isShift() && keyPress.keyBase == vsg::KEY_E &&
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
                    // Сесть на место машиниста = рабочее место:
                    // кабина с управлением (как вход через дверь)
                    _vehicles_handler->selectVehicle(hint_vehicle);

                    VehicleExterior* v = _vehicles_handler->getCurrentVehicle();

                    if (v != nullptr)
                        v->current_cabine_idx = hint_cab;

                    _prev_manipulator = nullptr;
                    _upd_server_control->setControlSuppressed(false);
                    _current_manipulator = _cabine_manipulator;
                    _current_manipulator->setCurrentVehicle(v);
                    _current_manipulator->resetView();
                }
                else
                {
                    // Сесть на место помощника: фиксированная камера
                    // на сиденье, управление НЕ берётся
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

""" + old_e
c = c.replace(old_e, new_e, 1)

# updateSeatHint реализация — перед tryEnterNearestCabine
old_try = """//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::tryEnterNearestCabine()"""
assert old_try in c
impl = """//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateViewerHandler::updateSeatHint()
{
    hint_vehicle = -1;

    if (gui_params != nullptr)
        gui_params->walk_hint.clear();

    if (_walk_manipulator->isSeated())
    {
        if (gui_params != nullptr)
            gui_params->walk_hint = (seated_driver) ?
                    QString("E — Встать с места машиниста") :
                    QString("E — Встать с места помощника");
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

    // Сиденье должно быть в конусе взгляда (~20 град) и близко
    const double max_cos = std::cos(vsg::radians(20.0));
    const double max_dist = 3.0;

    double best_cos = max_cos;

    for (size_t i = 0; i < vehicles.size(); ++i)
    {
        const VehicleExterior& vehicle = vehicles[i];

        // Место машиниста: driver_pos (сиденье - рабочая точка)
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

        // Место помощника
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

""" + old_try
c = c.replace(old_try, impl, 1)

open(r"viewer\src\UpdateViewerHandler.cpp", "w", encoding="utf-8").write(c)
print("handler ok, braces", c.count("{") - c.count("}"))
