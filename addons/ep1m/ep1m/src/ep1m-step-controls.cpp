#include    <ep1m.h>
#include    <ep1m-controls.h>

#include    "brake-crane.h"
#include    "loco-crane.h"
#include    "automatic-train-stop.h"
#include    "train-horn.h"
#include    "sanding-system.h"

#include    <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::stepControls(const double &t, const double &dt)
{
    (void) t;
    (void) dt;

    for (auto cab_idx : {CAB1, CAB2})
    {
        const int hold_buttons[] = {
            CTRL_EP1M_TUMBLER_RETURN_PROTECTION,
            CTRL_EP1M_BUTTON_RB,
            CTRL_EP1M_BUTTON_RBS,
            CTRL_EP1M_BUTTON_TYPHON,
            CTRL_EP1M_BUTTON_WHISTLE,
            CTRL_EP1M_BUTTON_SAND
        };

        for (int id : hold_buttons)
        {
            if (control_inputs[cab_idx].contains(id) &&
                (control_inputs[cab_idx].value(id) > 0.5f))
            {
                applyControlCommand(cab_idx, id, 1.0f);
            }
        }

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
void EP1m::applyControlCommand(int cab_idx, int id, float value)
{
    bool state = (value > 0.5f);

    switch (id)
    {
    case CTRL_EP1M_TUMBLER_MSUD:
    case CTRL_EP1M_TUMBLER_LOCK_VVK:
    case CTRL_EP1M_TUMBLER_PANT1:
    case CTRL_EP1M_TUMBLER_PANT2:
    case CTRL_EP1M_TUMBLER_RETURN_PROTECTION:
    case CTRL_EP1M_TUMBLER_MAIN_SWITCH:
    case CTRL_EP1M_TUMBLER_AUX_MACHINES:
    case CTRL_EP1M_TUMBLER_COMPRESSOR:
    case CTRL_EP1M_TUMBLER_FAN1:
    case CTRL_EP1M_TUMBLER_FAN2:
    case CTRL_EP1M_TUMBLER_FAN3:
    case CTRL_EP1M_TUMBLER_EPT:
    {
        if (tumblers_panel[cab_idx] != nullptr)
        {
            size_t idx = static_cast<size_t>(id - CTRL_EP1M_TUMBLER_MSUD);
            tumblers_panel[cab_idx]->setTumblerState(idx, state);
        }
        return;
    }

    case CTRL_EP1M_PANEL_KEY:
    {
        if (tumblers_panel[cab_idx] == nullptr)
        {
            return;
        }

        int phase = static_cast<int>(std::lround(value));

        if (phase >= 1)
        {
            tumblers_panel[cab_idx]->allowKey(true);
            tumblers_panel[cab_idx]->insertKey(true);
            tumblers_panel[cab_idx]->setKeyOn(phase >= 2);
        }
        else
        {
            tumblers_panel[cab_idx]->setKeyOn(false);
            tumblers_panel[cab_idx]->insertKey(false);
        }
        return;
    }

    case CTRL_EP1M_TUMBLER_MPK:
        state ? tumblers[TUMBLER_MPK][cab_idx].set()
              : tumblers[TUMBLER_MPK][cab_idx].reset();
        return;

    case CTRL_EP1M_TUMBLER_AUTO_MODE:
        state ? tumblers[TUMBLER_AUTO_MODE][cab_idx].set()
              : tumblers[TUMBLER_AUTO_MODE][cab_idx].reset();
        return;

    case CTRL_EP1M_TUMBLER_BUFFERLIGHT_L:
        state ? tumblers[TUMBLER_BUFFERLIGHT_L][cab_idx].set()
              : tumblers[TUMBLER_BUFFERLIGHT_L][cab_idx].reset();
        return;

    case CTRL_EP1M_TUMBLER_BUFFERLIGHT_R:
        state ? tumblers[TUMBLER_BUFFERLIGHT_R][cab_idx].set()
              : tumblers[TUMBLER_BUFFERLIGHT_R][cab_idx].reset();
        return;

    case CTRL_EP1M_TUMBLER_BUFFERCOLOR_L:
        state ? tumblers[TUMBLER_BUFFERCOLOR_L][cab_idx].set()
              : tumblers[TUMBLER_BUFFERCOLOR_L][cab_idx].reset();
        return;

    case CTRL_EP1M_TUMBLER_BUFFERCOLOR_R:
        state ? tumblers[TUMBLER_BUFFERCOLOR_R][cab_idx].set()
              : tumblers[TUMBLER_BUFFERCOLOR_R][cab_idx].reset();
        return;

    case CTRL_EP1M_TUMBLER_SPOTLIGHT_LOW:
        state ? tumblers[TUMBLER_SPOTLIGHT_LOW][cab_idx].set()
              : tumblers[TUMBLER_SPOTLIGHT_LOW][cab_idx].reset();
        return;

    case CTRL_EP1M_TUMBLER_SPOTLIGHT_HIGH:
        state ? tumblers[TUMBLER_SPOTLIGHT_HIGH][cab_idx].set()
              : tumblers[TUMBLER_SPOTLIGHT_HIGH][cab_idx].reset();
        return;

    case CTRL_EP1M_TUMBLER_CAB_LIGHT_LOW:
        state ? tumblers[TUMBLER_CAB_LIGHT_LOW][cab_idx].set()
              : tumblers[TUMBLER_CAB_LIGHT_LOW][cab_idx].reset();
        return;

    case CTRL_EP1M_TUMBLER_CAB_LIGHT_HIGH:
        state ? tumblers[TUMBLER_CAB_LIGHT_HIGH][cab_idx].set()
              : tumblers[TUMBLER_CAB_LIGHT_HIGH][cab_idx].reset();
        return;

    case CTRL_EP1M_TUMBLER_CAB_LIGHT_GREEN:
        state ? tumblers[TUMBLER_CAB_LIGHT_GREEN][cab_idx].set()
              : tumblers[TUMBLER_CAB_LIGHT_GREEN][cab_idx].reset();
        return;

    case CTRL_EP1M_TUMBLER_DEVICES_LIGHT:
        state ? tumblers[TUMBLER_DEVICES_LIGHT][cab_idx].set()
              : tumblers[TUMBLER_DEVICES_LIGHT][cab_idx].reset();
        return;

    case CTRL_EP1M_TUMBLER_BATTERY_OR_EPB:
        state ? tumblers[TUMBLER_BATTERY_OR_EPB_VOLTAGE][cab_idx].set()
              : tumblers[TUMBLER_BATTERY_OR_EPB_VOLTAGE][cab_idx].reset();
        return;

    case CTRL_EP1M_TUMBLER_SIGNAL_PANEL:
        state ? tumblers[TUMBLER_SIGNAL_PANEL_BS_002][cab_idx].set()
              : tumblers[TUMBLER_SIGNAL_PANEL_BS_002][cab_idx].reset();
        return;

    case CTRL_EP1M_TUMBLER_PCHF:
        state ? tumblers[TUMBLER_PCHF][cab_idx].set()
              : tumblers[TUMBLER_PCHF][cab_idx].reset();
        return;

    case CTRL_EP1M_CRANE_395:
        if (brake_crane[cab_idx] != nullptr)
        {
            brake_crane[cab_idx]->setHandlePosition(
                        static_cast<int>(std::lround(value)));
        }
        return;

    case CTRL_EP1M_CRANE_254:
        if (loco_crane[cab_idx] != nullptr)
        {
            loco_crane[cab_idx]->setHandlePosition(static_cast<double>(value));
        }
        return;

    case CTRL_EP1M_KM_LEVEL:
        if (km[cab_idx] != nullptr)
        {
            km[cab_idx]->setLevel(static_cast<double>(value));
        }
        return;

    case CTRL_EP1M_KM_MODE:
        if (km[cab_idx] != nullptr)
        {
            km[cab_idx]->setMode(static_cast<int8_t>(std::lround(value)));
        }
        return;

    case CTRL_EP1M_KM_REVERS:
    {
        if (km[cab_idx] == nullptr)
        {
            return;
        }

        switch (static_cast<int>(std::lround(value)))
        {
        case -1:
            km[cab_idx]->setReversBwd();
            break;

        case 1:
            km[cab_idx]->setReversFwd();
            break;

        default:
            km[cab_idx]->setReversZero();
            break;
        }
        return;
    }

    case CTRL_EP1M_KM_REVERS_INSERT:
        if (km[cab_idx] != nullptr)
        {
            km[cab_idx]->allowReversHandle(true);
            km[cab_idx]->insertReversHandle(value > 0.5f);
        }
        return;

    case CTRL_EP1M_LOCK_367:
        if (brake_lock[cab_idx] != nullptr)
        {
            brake_lock[cab_idx]->setStateOn(state);
        }
        return;

    case CTRL_EP1M_COMBINE_CRANE:
        if (brake_lock[cab_idx] != nullptr)
        {
            brake_lock[cab_idx]->setCombineCranePosition(
                        static_cast<int>(std::lround(value)));
        }
        return;

    case CTRL_EP1M_EPK_KEY:
    {
        if (epk[cab_idx] == nullptr)
        {
            return;
        }

        int phase = static_cast<int>(std::lround(value));

        if (phase >= 1)
        {
            epk[cab_idx]->allowKey(true);
            epk[cab_idx]->insertKey(true);
            epk[cab_idx]->setKeyOn(phase >= 2);
        }
        else
        {
            epk[cab_idx]->setKeyOn(false);
        }
        return;
    }

    case CTRL_EP1M_BUTTON_RB:
        state ? tumblers[BUTTON_RB][cab_idx].set()
              : tumblers[BUTTON_RB][cab_idx].reset();
        return;

    case CTRL_EP1M_BUTTON_RBS:
        state ? tumblers[BUTTON_RBS][cab_idx].set()
              : tumblers[BUTTON_RBS][cab_idx].reset();
        return;

    case CTRL_EP1M_BUTTON_TYPHON:
        if (horn[cab_idx] != nullptr)
        {
            horn[cab_idx]->setTifonOn(state);
        }
        return;

    case CTRL_EP1M_BUTTON_WHISTLE:
        if (horn[cab_idx] != nullptr)
        {
            horn[cab_idx]->setSvistokOn(state);
        }
        return;

    case CTRL_EP1M_BUTTON_SAND:
        sand_system->setSandDeliveryOn(state);
        return;

    default:
        return;
    }
}
