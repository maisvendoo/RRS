#include    <CabMouseHandler.h>

#include    <ProcVisibleAnimation.h>

#include    <VehiclesHandler.h>
#include    <VehicleExterior.h>
#include    <PlatformInput.h>
#include    <Logger.h>

#include    <vsg/app/Camera.h>
#include    <vsg/utils/LineSegmentIntersector.h>

#include    <algorithm>
#include    <cmath>

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

                // Моментальные кнопки (тифон, свисток, песок, РБ...):
                // удержание мыши = удержание кнопки, отпускание шлёт 0
                if (input.type == "Button")
                {
                    _held_button_ctrl = io_ctrl;
                    _held_button_name = input.contolledObjectName;
                }

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
void CabMouseHandler::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    if (_held_button_ctrl != nullptr)
    {
        _held_button_ctrl->mouseRelease(_held_button_name);
        _held_button_ctrl = nullptr;
        _held_button_name.clear();
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

    return held || isAltPhysicallyPressed();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CabMouseHandler::trySyntheticControl(IOController *ctrl,
                                          const std::string &node_name,
                                          float local_x,
                                          float local_y,
                                          IOController *&io_ctrl,
                                          io_control_input_t &input) const
{
    const QString synth_name =
            ctrl->pickSyntheticControl(node_name, local_x, local_y);

    if (synth_name.isEmpty())
    {
        return false;
    }

    auto synth_input = ctrl->getInputByObject(synth_name);

    if (!synth_input.has_value())
    {
        return false;
    }

    io_ctrl = ctrl;
    input = synth_input.value();

    return true;
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

    for (int pass = 0; pass < 2; ++pass)
    {
        // Второй проход: временно показываем скрытые анимации видимости -
        // спрятанные ключи и рукоятки должны кликаться по их месту в гнезде
        if (pass == 1)
        {
            ProcVisibleAnimation::forceShowAllHidden();
        }

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
                    if (ctrl == nullptr)
                    {
                        continue;
                    }

                    if (ctrl->findControl(node_name, input) && (input.id != 0))
                    {
                        io_ctrl = ctrl;
                    }
                    else if (trySyntheticControl(ctrl,
                                                 node_name,
                                                 static_cast<float>(hit->localIntersection.x),
                                                 static_cast<float>(hit->localIntersection.y),
                                                 io_ctrl,
                                                 input))
                    {
                    }
                    else
                    {
                        continue;
                    }

                    if (_last_hit_object != node_name)
                    {
                        _last_hit_object = node_name;
                        LOG_INFO("CabPick HIT: %s @(%d %d)", node_name.c_str(), x, y);
                    }

                    if (pass == 1)
                    {
                        ProcVisibleAnimation::restoreAllHidden();
                    }

                    return true;
                }
            }
        }
    }

        if (pass == 1)
        {
            ProcVisibleAnimation::restoreAllHidden();
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
            tip.state_text = io_ctrl->getControlStateText(input, state)
                    .toStdString();
        }
    }
}
