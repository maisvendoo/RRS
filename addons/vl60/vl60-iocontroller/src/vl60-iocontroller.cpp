#include    <algorithm>

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
    // Управление токоприемниками
    processTumbler(cabine_idx, CTRL_TUMBLER_PNT, pressed_keys);
    // Передний токоприемник
    processTumbler(cabine_idx, CTRL_TUMBLER_PNT1, pressed_keys);
    // Задний токоприемник
    processTumbler(cabine_idx, CTRL_TUMBLER_PNT2, pressed_keys);

    // Включение ГВ
    processTumbler(cabine_idx, CTRL_MAIN_SWITCH_ON, pressed_keys);
    // Возврат защиты ГВ
    processButton(cabine_idx, CTRL_RETURN_PROTECTION, pressed_keys);

    // Фазорасщепитель
    processTumbler(cabine_idx, CTRL_TUMBLER_FR, pressed_keys);

    // Компрессор
    processTumbler(cabine_idx, CTRL_TUMBLER_MK, pressed_keys);

    // Вентиляторы
    processTumbler(cabine_idx, CTRL_TUMBLER_MV1, pressed_keys);
    processTumbler(cabine_idx, CTRL_TUMBLER_MV2, pressed_keys);
    processTumbler(cabine_idx, CTRL_TUMBLER_MV3, pressed_keys);
    processTumbler(cabine_idx, CTRL_TUMBLER_MV4, pressed_keys);
    processTumbler(cabine_idx, CTRL_TUMBLER_MV5, pressed_keys);
    processTumbler(cabine_idx, CTRL_TUMBLER_MV6, pressed_keys);

    // Цепи управления
    processTumbler(cabine_idx, CTRL_TUMBLER_CU, pressed_keys);
    // Прожектор яркий
    processTumbler(cabine_idx, CTRL_TUMBLER_SPOT_HIGH, pressed_keys);
    // Прожектор тусклый
    processTumbler(cabine_idx, CTRL_TUMBLER_SPOT_LOW, pressed_keys);

    // Освещение кабины тусклое
    processTumbler(cabine_idx, CTRL_TUMBLER_CAB_LIGHT_LOW, pressed_keys);
    // Освещение кабины яркое
    processTumbler(cabine_idx, CTRL_TUMBLER_CAB_LIGHT_HIGH, pressed_keys);
    // Освещение приборов
    processTumbler(cabine_idx, CTRL_TUMBLER_LIGHT_DEVICES, pressed_keys);
    // Фонарь буферный левый
    processTumbler(cabine_idx, CTRL_TUMBLER_BUF_LIGHT_L, pressed_keys);
    // Фонарь буферный правый
    processTumbler(cabine_idx, CTRL_TUMBLER_BUF_LIGHT_R, pressed_keys);
    // Цвет буфера левый
    processTumbler(cabine_idx, CTRL_TUMBLER_BUF_COLOR_L, pressed_keys);
    // Цвет буфера правый
    processTumbler(cabine_idx, CTRL_TUMBLER_BUF_COLOR_R, pressed_keys);

    // ЭПТ
    processTumbler(cabine_idx, CTRL_TUMBLER_EPB, pressed_keys);

    // Установка и извление реверсивки
    processTumbler(cabine_idx, CTRL_REVERS_INSERTION, pressed_keys);

    // Извлечь/вставить ключ ЭПК-150
    processTumbler(cabine_idx, CTRL_EPK_INSERTION, pressed_keys);
    // Поворот ключа ЭПК-150
    processTumbler(cabine_idx, CTRL_KEY_EPK, pressed_keys);

    // Рукоятка блокировки 367
    processTumbler(cabine_idx, CTRL_LOCK_367_INSERTION, pressed_keys);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

int VL60IOController::kmPosBySignal(float signal) const
{
    if (signal > 1.1f)
    {
        return static_cast<int>(signal * 5.0f + 2.0f + 0.5f);
    }

    if (signal < -0.9f)  return 0;
    if (signal < -0.1f)  return 1;
    if (signal < 0.1f)   return 2;
    if (signal < 0.3f)   return 3;
    if (signal < 0.5f)   return 4;
    if (signal < 0.7f)   return 5;
    if (signal < 0.9f)   return 6;
    return 7;
}


void VL60IOController::processMouseInput(io_control_input_t input, uint32_t button, bool is_pressed)
{
    if (!is_pressed)
    {
        return;
    }

    const bool primary = (button == IO_CTRL_LEFT_MOUSE_BUTTON);
    const float cur = getSignalValueByName(input.contolledObjectName);

    const size_t cab = (input.cabine_idx >= 0) ? static_cast<size_t>(input.cabine_idx) : 0;

    auto send_pos = [&](float pos)
    {
        io_control_input_t out = input;
        out.value = pos;

        if (cab < io_control_inputs.size())
        {
            auto stored = io_control_inputs[cab].getByKey1(input.id);

            if (stored.has_value())
            {
                stored.value().value = pos;
                io_control_inputs[cab].updateByKey1(input.id, stored.value());
            }
        }

        emit sigSendVehicleControlCommand(out.serialize());
    };

    if (input.type == "Crane395")
    {
        int pos = (cur > 1.5f) ? static_cast<int>(cur + 0.5f) : static_cast<int>(cur * 6.0f + 0.5f);
        pos += primary ? 1 : -1;
        pos = std::clamp(pos, 0, 6);
        send_pos(static_cast<float>(pos));
        return;
    }

    if (input.type == "Crane254")
    {
        if (cur < -0.1f)
        {
            return;
        }

        const float step = 0.125f;
        float pos = cur + (primary ? step : -step);
        pos = std::clamp(pos, -0.05f, 1.0f);
        send_pos(pos);
        return;
    }

    if (input.type == "KM")
    {
        if (cur < -1.5f)
        {
            return;
        }

        int pos = kmPosBySignal(cur);
        pos += primary ? 1 : -1;
        pos = std::clamp(pos, 0, 33);
        send_pos(static_cast<float>(pos));
        return;
    }

    if (input.type == "Revers")
    {
        if (cab < io_control_inputs.size())
        {
            auto ins = io_control_inputs[cab].getByKey1(CTRL_REVERS_INSERTION);

            if (ins.has_value())
            {
                ins.value().value = 1.0f;
                io_control_inputs[cab].updateByKey1(CTRL_REVERS_INSERTION, ins.value());
                emit sigSendVehicleControlCommand(ins.value().serialize());
            }
        }

        if (cur < -2.5f)
        {
            return;
        }

        int pos = (cur < -1.0f) ? 0 : (cur > 1.0f) ? 2 : 1;
        pos += primary ? 1 : -1;
        pos = std::clamp(pos, 0, 2);
        send_pos(static_cast<float>(pos));
        return;
    }

    if (input.type == "Lock367")
    {
        send_pos((cur < 0.5f) ? 1.0f : 0.0f);
        return;
    }

    if (input.type == "Lever")
    {
        float pos = cur + (primary ? 1.0f : -1.0f);
        pos = std::clamp(pos, -1.0f, 1.0f);
        send_pos(pos);
        return;
    }
}



GET_MODULE(VL60IOController)
