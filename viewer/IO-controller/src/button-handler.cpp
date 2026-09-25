#include    <button-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ButtonHandler::ButtonHandler(QObject *parent) : ControlHandler(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ButtonHandler::processKeyInput(const std::set<uint16_t> &pressed_keys,
                                    int cabine_idx,
                                    int vehicle_idx)
{
    if (!ctrl_inputs)
    {
        return;
    }

    for (auto& [id, _, input] : (*ctrl_inputs)[cabine_idx].getAll())
    {
        if (getKeyState(pressed_keys, input.keyCode))
        {
            if (isKeyModifier(pressed_keys, input.keyModOnName) ||
                input.keyModOnName.isEmpty())
            {
                input.value = 1.0f;
            }
        }
        else
        {
            input.value = 0.0f;
        }

        sendControlSignal(input);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ButtonHandler::processMouseInput(const io_control_input_t &input,
                                      uint32_t button,
                                      bool is_pressed)
{
    if (!ctrl_inputs)
    {
        return;
    }

    auto io_ctrl = (*ctrl_inputs)[input.cabine_idx].getByKey1(input.id);
    if (!io_ctrl) return;

    if (is_pressed && button == IO_CTRL_LEFT_MOUSE_BUTTON)
    {
        io_ctrl->value = 1.0f;
        sendControlSignal(io_ctrl.value());
    }
    else if (!is_pressed && button == IO_CTRL_LEFT_MOUSE_BUTTON)
    {
        io_ctrl->value = 0.0f;
        sendControlSignal(io_ctrl.value());
    }
}

