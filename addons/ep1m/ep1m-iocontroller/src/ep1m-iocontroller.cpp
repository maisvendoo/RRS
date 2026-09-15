#include    <key-symbols.h>
#include    <ep1m-iocontroller.h>
#include    <ep1m-controls.h>
#include    <core/get_module.h>

#include    <algorithm>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EP1MIOController::EP1MIOController() : IOController(nullptr)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1MIOController::keysProcess(std::set<uint16_t> &pressed_keys)
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
void EP1MIOController::processTumblers(const std::set<uint16_t> &pressed_keys)
{
    // Клавиатурные тумблеры пульта (серверные биндинги переведены на команды)
    processTumbler(CTRL_EP1M_TUMBLER_MPK, pressed_keys);
    processTumbler(CTRL_EP1M_TUMBLER_AUTO_MODE, pressed_keys);
    processTumbler(CTRL_EP1M_TUMBLER_BUFFERLIGHT_L, pressed_keys);
    processTumbler(CTRL_EP1M_TUMBLER_BUFFERLIGHT_R, pressed_keys);
    processTumbler(CTRL_EP1M_TUMBLER_BUFFERCOLOR_L, pressed_keys);
    processTumbler(CTRL_EP1M_TUMBLER_BUFFERCOLOR_R, pressed_keys);
    processTumbler(CTRL_EP1M_TUMBLER_SPOTLIGHT_LOW, pressed_keys);
    processTumbler(CTRL_EP1M_TUMBLER_SPOTLIGHT_HIGH, pressed_keys);
    processTumbler(CTRL_EP1M_TUMBLER_CAB_LIGHT_LOW, pressed_keys);
    processTumbler(CTRL_EP1M_TUMBLER_CAB_LIGHT_HIGH, pressed_keys);
    processTumbler(CTRL_EP1M_TUMBLER_CAB_LIGHT_GREEN, pressed_keys);
    processTumbler(CTRL_EP1M_TUMBLER_DEVICES_LIGHT, pressed_keys);
    processTumbler(CTRL_EP1M_TUMBLER_BATTERY_OR_EPB, pressed_keys);
    processTumbler(CTRL_EP1M_TUMBLER_SIGNAL_PANEL, pressed_keys);
    processTumbler(CTRL_EP1M_TUMBLER_PCHF, pressed_keys);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1MIOController::processMouseControl(io_control_input_t &io_ctrl, int button)
{
    const bool primary = (button == 1);
    float cur = getVehicleSignal(io_ctrl.signal_id);

    // Тумблеры и 367 - в базовом классе (переключение по сигналу либо кэшу)
    if ((io_ctrl.type == "Toggle") || (io_ctrl.type == "Lock367"))
    {
        IOController::processMouseControl(io_ctrl, button);
        return;
    }

    // Моментальные кнопки (тифон, свисток, песок, РБ, РБС): нажатие
    // мышью = нажать и держать; отпускание (mouseRelease) шлёт 0
    if (io_ctrl.type == "Button")
    {
        io_ctrl.value = 1.0f;
        emitControl(io_ctrl);
        return;
    }

    // Ключ с фиксацией (панель тумблеров, ЭПК): 0 вынут / 1 вставлен /
    // 2 повёрнут. Клик идёт по циклу из кэша, SignalID - вставлен (0/1),
    // SignalID2 - повёрнут (0/1)
    if ((io_ctrl.type == "PanelKey") || (io_ctrl.type == "AutostopKey"))
    {
        const float inserted = getVehicleSignal(io_ctrl.signal_id);
        const float turned = getVehicleSignal(io_ctrl.signal_id2);

        float state = (turned > 0.5f) ? 2.0f :
                      ((inserted > 0.5f) ? 1.0f : io_ctrl.value);

        if ((inserted < 0.0f) && (io_ctrl.value > 0.5f))
        {
            state = io_ctrl.value;
        }

        state += primary ? 1.0f : -1.0f;
        io_ctrl.value = std::clamp(state, 0.0f, 2.0f);
        emitControl(io_ctrl);
        return;
    }

    if (io_ctrl.type == "Crane395")
    {
        // Сигнал нормализован 0..1 (позиция/6): ЛКМ - к экстренному
        if (cur < -0.5f)
        {
            return;
        }

        int pos = static_cast<int>((cur < 0.0f ? 0.0f : cur) * 6.0f + 0.5f);
        pos += primary ? 1 : -1;
        pos = std::clamp(pos, 0, 6);
        io_ctrl.value = static_cast<float>(pos);
        emitControl(io_ctrl);
        return;
    }

    if (io_ctrl.type == "Crane254")
    {
        // Целевое давление ТЦ: клик - ступень 0.5 кгс/см2 (0.125 хода)
        if (cur < -0.1f)
        {
            cur = io_ctrl.value;
        }

        const float step = 0.125f;
        float pos = cur + (primary ? step : -step);
        pos = std::clamp(pos, -0.05f, 1.0f);
        io_ctrl.value = pos;
        emitControl(io_ctrl);
        return;
    }

    if (io_ctrl.type == "KMLevel")
    {
        // Главная рукоятка КМ-35: уровень 0..1, шаг клика 5 позиций из 100.
        // Сигнал смешан с режимом - ведём локальный кэш
        float level = io_ctrl.value + (primary ? 0.05f : -0.05f);
        io_ctrl.value = std::clamp(level, 0.0f, 1.0f);
        emitControl(io_ctrl);
        return;
    }

    if (io_ctrl.type == "Revers")
    {
        // Реверс: если рукоятка не вставлена (SignalID2) - ЛКМ вставляет
        // её, ПКМ извлекает в нейтрали
        const float inserted = getVehicleSignal(io_ctrl.signal_id2);

        if ((inserted >= 0.0f) && (inserted < 0.5f))
        {
            if (primary)
            {
                auto ins = io_control_inputs.getByKey1(CTRL_EP1M_KM_REVERS_INSERT);

                if (ins.has_value())
                {
                    ins.value().value = 1.0f;
                    emitControl(ins.value());
                }
            }
            return;
        }

        // Сигнал -1/0/1 (источник истины), при недоступности - кэш.
        // ЛКМ - к вперёд, ПКМ - к назад
        float pos = ((cur >= -1.5f) && (cur <= 1.5f)) ? cur : io_ctrl.value;
        pos += primary ? 1.0f : -1.0f;
        io_ctrl.value = std::clamp(pos, -1.0f, 1.0f);
        emitControl(io_ctrl);
        return;
    }

    if (io_ctrl.type == "KMMode")
    {
        // Режимная рукоятка: -1 / 0 / 1 (кэш)
        float pos = io_ctrl.value + (primary ? 1.0f : -1.0f);
        io_ctrl.value = std::clamp(pos, -1.0f, 1.0f);
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
GET_MODULE(EP1MIOController)
