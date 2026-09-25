#include    <toggle-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ToggleHandler::ToggleHandler(QObject *parent) : ControlHandler(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ToggleHandler::processKeyInput(const std::set<uint16_t> &pressed_keys,
                                    int cabine_idx,
                                    int vehicle_idx)
{
    if (!ctrl_inputs) return;

    for (const auto& [id, _, input] : (*ctrl_inputs)[cabine_idx].getAll())
    {
        if (input.type == "Toggle")
            processTumbler(cabine_idx, id, pressed_keys);
        else if (input.type == "Button")
            processButton(cabine_idx, id, pressed_keys);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ToggleHandler::processMouseInput(const io_control_input_t &input,
                                      uint32_t button,
                                      bool is_pressed)
{
    if (!ctrl_inputs) return;

    auto io_ctrl = (*ctrl_inputs)[input.cabine_idx].getByKey1(input.id);
    if (!io_ctrl) return;

    if (io_ctrl->type == "Toggle")
    {
        if (button == IO_CTRL_LEFT_MOUSE_BUTTON && !input.toBool())
        {
            io_ctrl->value = 1.0f;
            sendControlSignal(io_ctrl->controlled_vehicle_idx,
                              input.cabine_idx, input.id, io_ctrl->value);
        }

        if (button == IO_CTRL_RIGHT_MOUSE_BUTTON && input.toBool())
        {
            io_ctrl->value = 0.0f;
            sendControlSignal(io_ctrl->controlled_vehicle_idx,
                              input.cabine_idx, input.id, io_ctrl->value);
        }
    }
    else if (io_ctrl->type == "Button")
    {
        if (is_pressed && button == IO_CTRL_LEFT_MOUSE_BUTTON)
        {
            io_ctrl->value = 1.0f;
            sendControlSignal(io_ctrl->controlled_vehicle_idx,
                              input.cabine_idx, input.id, io_ctrl->value);
        }
        else if (!is_pressed && button == IO_CTRL_LEFT_MOUSE_BUTTON)
        {
            io_ctrl->value = 0.0f;
            sendControlSignal(io_ctrl->controlled_vehicle_idx,
                              input.cabine_idx, input.id, io_ctrl->value);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ToggleHandler::processTumbler(size_t cab_idx,
                                   uint16_t control_id,
                                   const std::set<uint16_t> &pressed_keys)
{
    if (!ctrl_inputs) return;

    auto io_ctrl = (*ctrl_inputs)[cab_idx].getByKey1(control_id);
    if (!io_ctrl || io_ctrl->type != "Toggle") return;

    if (getKeyState(pressed_keys, io_ctrl->keyCode))
    {
        if (io_ctrl->keyModOnName == io_ctrl->keyModOffName)
        {
            if (isKeyModifier(pressed_keys, io_ctrl->keyModOnName))
            {
                io_ctrl->value = 1.0f - io_ctrl->value;
                sendControlSignal(io_ctrl->controlled_vehicle_idx, cab_idx,
                                  control_id, io_ctrl->value);
                return;
            }
        }

        if (isKeyModifier(pressed_keys, io_ctrl->keyModOnName))
        {
            io_ctrl->value = 1.0f;
            sendControlSignal(io_ctrl->controlled_vehicle_idx, cab_idx,
                              control_id, io_ctrl->value);
            return;
        }

        if (isKeyModifier(pressed_keys, io_ctrl->keyModOffName))
        {
            io_ctrl->value = 0.0f;
            sendControlSignal(io_ctrl->controlled_vehicle_idx, cab_idx,
                              control_id, io_ctrl->value);
            return;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ToggleHandler::processButton(size_t cab_idx,
                                  uint16_t control_id,
                                  const std::set<uint16_t> &pressed_keys)
{
    if (!ctrl_inputs) return;

    auto io_ctrl = (*ctrl_inputs)[cab_idx].getByKey1(control_id);
    if (!io_ctrl || io_ctrl->type != "Button") return;

    if (getKeyState(pressed_keys, io_ctrl->keyCode))
    {
        if (isKeyModifier(pressed_keys, io_ctrl->keyModOnName) ||
            io_ctrl->keyModOnName.isEmpty())
        {
            io_ctrl->value = 1.0f;
        }
    }
    else
    {
        io_ctrl->value = 0.0f;
    }

    sendControlSignal(io_ctrl->controlled_vehicle_idx, cab_idx,
                      control_id, io_ctrl->value);
}
