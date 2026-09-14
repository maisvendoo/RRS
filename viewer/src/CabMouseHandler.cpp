#include    <CabMouseHandler.h>

#include    <VehiclesHandler.h>
#include    <VehicleExterior.h>
#include    <Logger.h>

#include    <vsg/app/Camera.h>
#include    <vsg/utils/LineSegmentIntersector.h>

#include    <algorithm>
#include    <cmath>

#ifdef _WIN32
    #include <windows.h>
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CabTooltipState& cabTooltip()
{
    static CabTooltipState state;
    return state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CabMouseHandler::CabMouseHandler(vsg::ref_ptr<vsg::Camera> camera,
                                 vsg::ref_ptr<vsg::Keyboard> keyboard,
                                 VehiclesHandler* vehicles_handler)
    : _camera(camera)
    , _keyboard(keyboard)
    , _vehicles_handler(vehicles_handler)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CabMouseHandler::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }

    // ЛКМ/ПКМ по органу кабины - команда управления через IOController.
    // Alt не нужен - он только для подсказок
    if ((buttonPress.button == 1) || (buttonPress.button == 3))
    {
        IOController* io_ctrl = nullptr;
        io_control_input_t input;

        if (pickControl(static_cast<int>(buttonPress.x),
                        static_cast<int>(buttonPress.y),
                        io_ctrl, input))
        {
            if (input.id != 0)
            {
                LOG_INFO("CabClick: %s (id=%u) -> IOController command",
                         input.contolledObjectName.toStdString().c_str(),
                         static_cast<unsigned>(input.id));

                io_ctrl->mouseClick(input.contolledObjectName,
                                    static_cast<int>(buttonPress.button));

                // Клик по органу не должен крутить камеру
                buttonPress.handled = true;
                return;
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CabMouseHandler::apply(vsg::MoveEvent& moveEvent)
{
    _pointer_x = moveEvent.x;
    _pointer_y = moveEvent.y;
    _pointer_valid = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CabMouseHandler::apply(vsg::FrameEvent& frame)
{
    (void) frame;

    if (altHeld() && _pointer_valid)
    {
        updateTooltip();
    }
    else if (cabTooltip().active)
    {
        cabTooltip().active = false;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CabMouseHandler::altHeld() const
{
    const bool held = (_keyboard != nullptr) &&
            (_keyboard->pressed(vsg::KEY_Alt_L) || _keyboard->pressed(vsg::KEY_Alt_R));

#ifdef _WIN32
    // Alt приходит как WM_SYSKEYDOWN и не всегда попадает в vsg::Keyboard -
    // дублируем опросом физического состояния (Windows)
    if (!held)
    {
        return (::GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
    }
#endif

    return held;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CabMouseHandler::pickControl(int x, int y,
                                  IOController*& io_ctrl,
                                  io_control_input_t& input)
{
    io_ctrl = nullptr;

    VehicleExterior* veh = _vehicles_handler->getCurrentVehicle();

    if ((veh == nullptr) || veh->io_controls.empty() || (_camera == nullptr))
    {
        return false;
    }

    // Луч камера->курсор: штатная реализация vsg (viewport+projection).
    // Малые органы (тумблеры) легко промазать между наведением и кликом -
    // кидаем веер лучей вокруг курсора, первый элемент выигрывает
    static const int offsets[5][2] = {
        {0, 0}, {9, 0}, {-9, 0}, {0, 9}, {0, -9}
    };

    vsg::ref_ptr<vsg::LineSegmentIntersector> intersector;

    for (auto& off : offsets)
    {
        intersector = vsg::LineSegmentIntersector::create(*_camera, x + off[0], y + off[1]);
        veh->transform->accept(*intersector);

        for (const auto& hit : intersector->intersections)
        {
            for (auto node_it = hit->nodePath.rbegin(); node_it != hit->nodePath.rend(); ++node_it)
            {
                std::string node_name = "";
                (*node_it)->getValue("name", node_name);

                if (node_name.empty())
                {
                    (*node_it)->getValue("Name", node_name);
                }

                if (node_name.empty())
                {
                    continue;
                }

                for (auto* ctrl : veh->io_controls)
                {
                    if ((ctrl != nullptr) && ctrl->findControl(node_name, input))
                    {
                        io_ctrl = ctrl;

                        if (_last_hit_object != node_name)
                        {
                            _last_hit_object = node_name;
                            LOG_INFO("CabPick HIT: %s @(%d %d)", node_name.c_str(), x, y);
                        }

                        return true;
                    }
                }
            }
        }
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CabMouseHandler::updateTooltip()
{
    CabTooltipState& tip = cabTooltip();

    IOController* io_ctrl = nullptr;
    io_control_input_t input;

    if (!pickControl(static_cast<int>(_pointer_x),
                     static_cast<int>(_pointer_y),
                     io_ctrl, input))
    {
        tip.active = false;
        return;
    }

    tip.active = true;
    tip.x = _pointer_x;
    tip.y = _pointer_y;
    tip.title = input.name.toStdString();
    tip.state_text = "";
    tip.action_text = u8"не управляется мышью";

    if (input.id != 0)
    {
        if ((input.type == "Toggle") || (input.type == "Button"))
        {
            tip.action_text = (input.type == "Button")
                    ? u8"ЛКМ - нажать"
                    : u8"ЛКМ - переключить";
        }
        else
        {
            tip.action_text = u8"ЛКМ - вперёд, ПКМ - назад";
        }
    }

    if ((io_ctrl != nullptr) && (input.signal_id >= 0))
    {
        const float state = io_ctrl->getVehicleSignal(input.signal_id);

        if (state >= 0.0f)
        {
            if ((input.type == "Toggle") || (input.type == "Button"))
            {
                tip.state_text = (state > 0.5f)
                        ? u8"состояние: включено"
                        : u8"состояние: выключено";
            }
            else if (input.state_mode == "kme")
            {
                // КМЭ-60-044: сигнал нелинеен
                if (state < -0.9f)          tip.state_text = u8"положение: БВ - быстрое выключение";
                else if (state < -0.1f)     tip.state_text = u8"положение: Ноль";
                else if (state < 0.1f)      tip.state_text = u8"положение: АВ - автоматическое выключение";
                else if (state < 0.3f)      tip.state_text = u8"положение: РВ - ручное выключение";
                else if (state < 0.5f)      tip.state_text = u8"положение: ФВ - фиксация выключения";
                else if (state < 0.7f)      tip.state_text = u8"положение: ФП - фиксация пуска";
                else if (state < 0.9f)      tip.state_text = u8"положение: РП - ручной пуск";
                else if (state < 1.1f)      tip.state_text = u8"положение: АП - автоматический пуск";
                else
                {
                    const int pos = static_cast<int>(state * 5.0f - 5.0f + 0.5f);
                    char buf[64];
                    std::snprintf(buf, sizeof(buf), u8"положение: позиция %d",
                                  std::max(pos, 1));
                    tip.state_text = buf;
                }
            }
            else if (input.type == "Crane254")
            {
                // Рукоятка 0..1 задаёт целевое давление ТЦ (kvt254:
                // k1=0.4 МПа); показываем его в кгс/см² вместо процентов
                char buf[64];

                if (state < -0.01f)
                {
                    std::snprintf(buf, sizeof(buf), u8"целевое: отпускное (выпуск ТЦ)");
                }
                else
                {
                    const float p_target = state * 0.4f * 10.2f;
                    std::snprintf(buf, sizeof(buf), u8"целевое: %.1f кгс/см²", p_target);
                }

                tip.state_text = buf;
            }
            else if (!input.state_names.isEmpty())
            {
                const QStringList names =
                        input.state_names.split(';', Qt::KeepEmptyParts);

                if (!names.isEmpty())
                {
                    int idx = 0;

                    if (input.state_mode == "centered")
                    {
                        idx = (state < -0.5f) ? 0
                            : (state > 0.5f) ? static_cast<int>(names.size() - 1)
                            : static_cast<int>(names.size() / 2);
                    }
                    else if (input.state_mode == "index")
                    {
                        idx = std::min(static_cast<int>(names.size() - 1),
                                       static_cast<int>(state + 0.5f));
                    }
                    else
                    {
                        // norm: сигнал 0..1 (кран 395: позиция/6)
                        idx = std::min(static_cast<int>(names.size() - 1),
                                       static_cast<int>(state * names.size()));
                    }

                    idx = std::clamp(idx, 0, static_cast<int>(names.size() - 1));
                    tip.state_text = u8"положение: " + names[idx].toStdString();
                }
            }
            else
            {
                char buf[64];
                std::snprintf(buf, sizeof(buf), u8"положение: %d%%",
                              static_cast<int>(state * 100.0f));
                tip.state_text = buf;
            }
        }
    }
}
