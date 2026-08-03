#include    <key-symbols.h>
#include    <vl60-iocontroller.h>
#include    <vl60-controls.h>
#include    <core/get_module.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60IOController::VL60IOController() : IOController(nullptr)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60IOController::keysProcess(std::set<uint16_t> &pressed_keys)
{
    if (pressed_keys.empty())
    {
        return;
    }

    // Проверяем конкретный контрол
    auto io_ctrl = io_control_inputs.getByKey1(CTRL_TUMBLER_PNT);

    // Нажата ли его клавиша
    if (getKeyState(pressed_keys, io_ctrl->keyCode))
    {
        // Какой модификатор?
        if (isShift(pressed_keys))
        {
            io_ctrl->value = 1.0f;
        }

        if (isControl(pressed_keys))
        {
            io_ctrl->value = 0.0f;
        }

        emit sigSendVehicleControlCommand(io_ctrl->serialize());
    }

    pressed_keys.clear();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GET_MODULE(VL60IOController)
