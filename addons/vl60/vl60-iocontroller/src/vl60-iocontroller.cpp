#include    <key-symbols.h>
#include    <vl60-iocontroller.h>
#include    <vl60-controls.h>
#include    <core/get_module.h>

#include    <algorithm>

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

    processTumblers(pressed_keys);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60IOController::processTumblers(const std::set<uint16_t> &pressed_keys)
{
    // Тумблеры с клавишами включения/отключения (Shift+клавиша / Ctrl+клавиша)
    processTumbler(CTRL_TUMBLER_PNT, pressed_keys);
    processTumbler(CTRL_TUMBLER_PNT1, pressed_keys);
    processTumbler(CTRL_TUMBLER_PNT2, pressed_keys);
    processTumbler(CTRL_TUMBLER_CU, pressed_keys);
    processTumbler(CTRL_TUMBLER_GV, pressed_keys);
    processTumbler(CTRL_TUMBLER_FR, pressed_keys);
    processTumbler(CTRL_TUMBLER_MK, pressed_keys);
    processTumbler(CTRL_TUMBLER_MV1, pressed_keys);
    processTumbler(CTRL_TUMBLER_MV2, pressed_keys);
    processTumbler(CTRL_TUMBLER_MV3, pressed_keys);
    processTumbler(CTRL_TUMBLER_MV4, pressed_keys);
    processTumbler(CTRL_TUMBLER_MV5, pressed_keys);
    processTumbler(CTRL_TUMBLER_MV6, pressed_keys);
    processTumbler(CTRL_TUMBLER_EPT, pressed_keys);
    processTumbler(CTRL_TUMBLER_CAB_LIGHT_LOW, pressed_keys);
    processTumbler(CTRL_TUMBLER_LIGHT_DEVICES, pressed_keys);
    processTumbler(CTRL_TUMBLER_BUFFLIGHT_L, pressed_keys);
    processTumbler(CTRL_TUMBLER_BUFFLIGHT_R, pressed_keys);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int VL60IOController::kmPosBySignal(float signal) const
{
    // Кодирование kme-60-044: -1 БВ; -0.2 Ноль; 0 АВ; 0.2 РВ; 0.4 ФВ;
    // 0.6 ФП; 0.8 РП; 1.0 АП; выше - тяговые позиции (s*5+2)
    if (signal > 1.1f)
    {
        return static_cast<int>(signal * 5.0f + 2.0f + 0.5f);
    }

    if (signal < -0.9f)  return 0;  // БВ
    if (signal < -0.1f)  return 1;  // Ноль
    if (signal < 0.1f)   return 2;  // АВ
    if (signal < 0.3f)   return 3;  // РВ
    if (signal < 0.5f)   return 4;  // ФВ
    if (signal < 0.7f)   return 5;  // ФП
    if (signal < 0.9f)   return 6;  // РП
    return 7;                      // АП
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60IOController::processMouseControl(io_control_input_t &io_ctrl, int button)
{
    const bool primary = (button == 1);
    float cur = getVehicleSignal(io_ctrl.signal_id);

    // Тумблеры и кнопки - в базовом классе (переключение по сигналу)
    if ((io_ctrl.type == "Toggle") || (io_ctrl.type == "Button"))
    {
        IOController::processMouseControl(io_ctrl, button);
        return;
    }

    if (io_ctrl.type == "Crane395")
    {
        // Сигнал нормализован 0..1 (позиция/6): ЛКМ - к экстренному,
        // ПКМ - к отпуску
        if (cur < 0.0f)
        {
            return;
        }

        int pos = static_cast<int>(cur * 6.0f + 0.5f);
        pos += primary ? 1 : -1;
        pos = std::clamp(pos, 0, 6);
        io_ctrl.value = static_cast<float>(pos);
        emitControl(io_ctrl);
        return;
    }

    if (io_ctrl.type == "Crane254")
    {
        // Положение рукоятки 0..1 задаёт ЦЕЛЕВОЕ давление ТЦ
        // (kvt254: k1=0.4 МПа = 4.0 кгс/см² на полном ходе). Один клик -
        // ступень 0.5 кгс/см² (= 0.125 хода); ПКМ ниже нуля - отпускное
        if (cur < -0.1f)
        {
            return;
        }

        const float step = 0.125f;
        float pos = cur + (primary ? step : -step);
        pos = std::clamp(pos, -0.05f, 1.0f);
        io_ctrl.value = pos;
        emitControl(io_ctrl);
        return;
    }

    if (io_ctrl.type == "KM")
    {
        // Главная рукоятка: ЛКМ - позиция вверх, ПКМ - вниз
        if (cur < -1.5f)
        {
            return;
        }

        int pos = kmPosBySignal(cur);
        pos += primary ? 1 : -1;
        pos = std::clamp(pos, 0, 33);
        io_ctrl.value = static_cast<float>(pos);
        emitControl(io_ctrl);
        return;
    }

    if (io_ctrl.type == "Revers")
    {
        // Реверс: сигнал -2..+2 (назад/ноль/вперёд); рукоятка не
        // вставлена (SignalID2) - ЛКМ вставляет её
        const float inserted = getVehicleSignal(io_ctrl.signal_id2);

        if ((inserted >= 0.0f) && (inserted < 0.5f))
        {
            if (primary)
            {
                auto ins = io_control_inputs.getByKey1(CTRL_KM_REVERS_INSERT);
                if (ins.has_value())
                {
                    ins.value().value = 1.0f;
                    emitControl(ins.value());
                }
            }
            return;
        }

        if (cur < -2.5f)
        {
            return;
        }

        int pos = (cur < -1.0f) ? 0 : (cur > 1.0f) ? 2 : 1;
        pos += primary ? 1 : -1;
        pos = std::clamp(pos, 0, 2);
        io_ctrl.value = static_cast<float>(pos);
        emitControl(io_ctrl);
        return;
    }

    if (io_ctrl.type == "Lever")
    {
        // Комбинированный кран: -1 двойная тяга / 0 поездное / +1 экстренное
        if (cur < -1.5f)
        {
            cur = io_ctrl.value;
        }

        float pos = cur + (primary ? 1.0f : -1.0f);
        pos = std::clamp(pos, -1.0f, 1.0f);
        io_ctrl.value = pos;
        emitControl(io_ctrl);
        return;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GET_MODULE(VL60IOController)
