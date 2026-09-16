#include    <cmath>
#include    <vl60pk.h>
#include    <vl60-controls.h>

#include    "brake-crane.h"
#include    "loco-crane.h"
#include    "kme-60-044.h"
#include    "pneumo-brake-lock.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepControls(const double &t, const double &dt)
{
    (void) t;
    (void) dt;

    for (auto cab_idx : {CAB1, CAB2})
    {
        // Применяем только ИЗМЕНИВШИЕСЯ команды (по фронту): команда
        // задаёт целевое состояние органа, автозапуск/автоостанов
        // и прочие серверные программы работают с устройствами напрямую
        // и не должны затираться уровнем последней команды
        for (auto it = control_inputs[cab_idx].cbegin();
             it != control_inputs[cab_idx].cend(); ++it)
        {
            int id = it.key();
            float value = it.value();

            if (prev_control_values[cab_idx].contains(id) &&
                (prev_control_values[cab_idx].value(id) == value))
            {
                continue;
            }

            prev_control_values[cab_idx].insert(id, value);

            applyControlCommand(cab_idx, id, value);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::applyControlCommand(int cab_idx, int id, float value)
{
    bool state = (value > 0.5f);

    switch (id)
    {
    // Тумблеры: команда задаёт целевое состояние
    case CTRL_TUMBLER_PNT:
        state ? pants_tumbler[cab_idx].set() : pants_tumbler[cab_idx].reset();
        return;

    case CTRL_TUMBLER_PNT1:
        state ? pant1_tumbler[cab_idx].set() : pant1_tumbler[cab_idx].reset();
        return;

    case CTRL_TUMBLER_PNT2:
        state ? pant2_tumbler[cab_idx].set() : pant2_tumbler[cab_idx].reset();
        return;

    case CTRL_TUMBLER_CU:
        state ? cu_tumbler[cab_idx].set() : cu_tumbler[cab_idx].reset();
        return;

    case CTRL_TUMBLER_GV:
        state ? gv_tumbler[cab_idx].set() : gv_tumbler[cab_idx].reset();
        return;

    case CTRL_TUMBLER_GV_RETURN:
        // Кнопка без фиксации: нажатие - включить, отпускание (value=0) - выключить
        state ? gv_return_tumbler[cab_idx].set() : gv_return_tumbler[cab_idx].reset();
        return;

    case CTRL_TUMBLER_FR:
        state ? fr_tumbler[cab_idx].set() : fr_tumbler[cab_idx].reset();
        return;

    case CTRL_TUMBLER_MK:
        state ? mk_tumbler[cab_idx].set() : mk_tumbler[cab_idx].reset();
        return;

    case CTRL_TUMBLER_MV1:
    case CTRL_TUMBLER_MV2:
    case CTRL_TUMBLER_MV3:
    case CTRL_TUMBLER_MV4:
    case CTRL_TUMBLER_MV5:
    case CTRL_TUMBLER_MV6:
    {
        size_t mv_idx = static_cast<size_t>(id - CTRL_TUMBLER_MV1);
        state ? mv_tumblers[cab_idx][mv_idx].set()
              : mv_tumblers[cab_idx][mv_idx].reset();
        return;
    }

    case CTRL_TUMBLER_EPT:
        state ? epb_switch[cab_idx].set() : epb_switch[cab_idx].reset();
        return;

    case CTRL_TUMBLER_CAB_LIGHT_LOW:
        state ? P_cab_light_low_tumbler[cab_idx].set()
              : P_cab_light_low_tumbler[cab_idx].reset();
        return;

    case CTRL_TUMBLER_CAB_LIGHT_HIGH:
        state ? P_cab_light_high_tumbler[cab_idx].set()
              : P_cab_light_high_tumbler[cab_idx].reset();
        return;

    case CTRL_TUMBLER_LIGHT_DEVICES:
        state ? P_light_devices_tumbler[cab_idx].set()
              : P_light_devices_tumbler[cab_idx].reset();
        return;

    case CTRL_TUMBLER_BUFFLIGHT_L:
        state ? P_bufferlight_L_tumbler[cab_idx].set()
              : P_bufferlight_L_tumbler[cab_idx].reset();
        return;

    case CTRL_TUMBLER_BUFFLIGHT_R:
        state ? P_bufferlight_R_tumbler[cab_idx].set()
              : P_bufferlight_R_tumbler[cab_idx].reset();
        return;

    case CTRL_TUMBLER_BUFFCOLOR_L:
        state ? P_buffercolor_L_toogle[cab_idx].set()
              : P_buffercolor_L_toogle[cab_idx].reset();
        return;

    case CTRL_TUMBLER_BUFFCOLOR_R:
        state ? P_buffercolor_R_toogle[cab_idx].set()
              : P_buffercolor_R_toogle[cab_idx].reset();
        return;

    // Кран машиниста 395: позиция I..VI (0..6)
    case CTRL_CRANE_395:
        brake_crane[cab_idx]->setHandlePosition(
                    static_cast<int>(std::lround(value)));
        return;

    // Кран вспомогательный 254
    case CTRL_CRANE_254:
        loco_crane[cab_idx]->setHandlePosition(static_cast<double>(value));
        return;

    // КМЭ главная рукоятка: позиция 0..33
    case CTRL_KM_MAIN:
        controller[cab_idx]->setMainHandlePos(
                    static_cast<int>(std::lround(value)));
        return;

    // КМЭ реверс: 0 - назад, 1 - ноль, 2 - вперёд
    case CTRL_KM_REVERS:
        controller[cab_idx]->setReversHandlePos(
                    static_cast<int>(std::lround(value)));
        return;

    // Вставка/извлечение реверсивной рукоятки
    case CTRL_KM_REVERS_INSERT:
        controller[cab_idx]->insertReversHandle(value > 0.5f);
        return;

    // УБТ-367: поворот ключа блокировки
    case CTRL_LOCK_367:
        brake_lock[cab_idx]->setStateOn(value > 0.5f);
        return;

    // Комбинированный кран: -1 - двойная тяга, 0 - поездное, +1 - экстренное
    case CTRL_CRANE_COMBINE:
        brake_lock[cab_idx]->setCombineCranePosition(
                    static_cast<int>(std::lround(value)));
        return;

    default:
        return;
    }
}
