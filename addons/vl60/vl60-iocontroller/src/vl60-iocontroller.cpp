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
    // ╨в╤Г╨╝╨▒╨╗╨╡╤А╤Л ╤Б ╨║╨╗╨░╨▓╨╕╤И╨░╨╝╨╕ ╨▓╨║╨╗╤О╤З╨╡╨╜╨╕╤П/╨╛╤В╨║╨╗╤О╤З╨╡╨╜╨╕╤П (Shift+╨║╨╗╨░╨▓╨╕╤И╨░ / Ctrl+╨║╨╗╨░╨▓╨╕╤И╨░)
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
    // ╨Ъ╨╛╨┤╨╕╤А╨╛╨▓╨░╨╜╨╕╨╡ kme-60-044: -1 ╨С╨Т; -0.2 ╨Э╨╛╨╗╤М; 0 ╨Р╨Т; 0.2 ╨а╨Т; 0.4 ╨д╨Т;
    // 0.6 ╨д╨Я; 0.8 ╨а╨Я; 1.0 ╨Р╨Я; ╨▓╤Л╤И╨╡ - ╤В╤П╨│╨╛╨▓╤Л╨╡ ╨┐╨╛╨╖╨╕╤Ж╨╕╨╕ (s*5+2)
    if (signal > 1.1f)
    {
        return static_cast<int>(signal * 5.0f + 2.0f + 0.5f);
    }

    if (signal < -0.9f)  return 0;  // ╨С╨Т
    if (signal < -0.1f)  return 1;  // ╨Э╨╛╨╗╤М
    if (signal < 0.1f)   return 2;  // ╨Р╨Т
    if (signal < 0.3f)   return 3;  // ╨а╨Т
    if (signal < 0.5f)   return 4;  // ╨д╨Т
    if (signal < 0.7f)   return 5;  // ╨д╨Я
    if (signal < 0.9f)   return 6;  // ╨а╨Я
    return 7;                      // ╨Р╨Я
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60IOController::processMouseControl(io_control_input_t &io_ctrl, int button)
{
    const bool primary = (button == 1);
    float cur = getVehicleSignal(io_ctrl.signal_id);

    // ╨в╤Г╨╝╨▒╨╗╨╡╤А╤Л ╨╕ ╨║╨╜╨╛╨┐╨║╨╕ - ╨▓ ╨▒╨░╨╖╨╛╨▓╨╛╨╝ ╨║╨╗╨░╤Б╤Б╨╡ (╨┐╨╡╤А╨╡╨║╨╗╤О╤З╨╡╨╜╨╕╨╡ ╨┐╨╛ ╤Б╨╕╨│╨╜╨░╨╗╤Г)
    if ((io_ctrl.type == "Toggle") || (io_ctrl.type == "Button"))
    {
        IOController::processMouseControl(io_ctrl, button);
        return;
    }

    if (io_ctrl.type == "Lock367")
    {
        processSwitchBySignal(io_ctrl);
        return;
    }

    if (io_ctrl.type == "Crane395")
    {
        // ╨б╨╕╨│╨╜╨░╨╗ ╨╜╨╛╤А╨╝╨░╨╗╨╕╨╖╨╛╨▓╨░╨╜ 0..1 (╨┐╨╛╨╖╨╕╤Ж╨╕╤П/6): ╨Ы╨Ъ╨Ь - ╨║ ╤Н╨║╤Б╤В╤А╨╡╨╜╨╜╨╛╨╝╤Г,
        // ╨Я╨Ъ╨Ь - ╨║ ╨╛╤В╨┐╤Г╤Б╨║╤Г
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
        // ╨Я╨╛╨╗╨╛╨╢╨╡╨╜╨╕╨╡ ╤А╤Г╨║╨╛╤П╤В╨║╨╕ 0..1 ╨╖╨░╨┤╨░╤С╤В ╨ж╨Х╨Ы╨Х╨Т╨Ю╨Х ╨┤╨░╨▓╨╗╨╡╨╜╨╕╨╡ ╨в╨ж
        // (kvt254: k1=0.4 ╨Ь╨Я╨░ = 4.0 ╨║╨│╤Б/╤Б╨╝┬▓ ╨╜╨░ ╨┐╨╛╨╗╨╜╨╛╨╝ ╤Е╨╛╨┤╨╡). ╨Ю╨┤╨╕╨╜ ╨║╨╗╨╕╨║ -
        // ╤Б╤В╤Г╨┐╨╡╨╜╤М 0.5 ╨║╨│╤Б/╤Б╨╝┬▓ (= 0.125 ╤Е╨╛╨┤╨░); ╨Я╨Ъ╨Ь ╨╜╨╕╨╢╨╡ ╨╜╤Г╨╗╤П - ╨╛╤В╨┐╤Г╤Б╨║╨╜╨╛╨╡
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
        // ╨У╨╗╨░╨▓╨╜╨░╤П ╤А╤Г╨║╨╛╤П╤В╨║╨░: ╨Ы╨Ъ╨Ь - ╨┐╨╛╨╖╨╕╤Ж╨╕╤П ╨▓╨▓╨╡╤А╤Е, ╨Я╨Ъ╨Ь - ╨▓╨╜╨╕╨╖
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
        // ╨а╨╡╨▓╨╡╤А╤Б: ╤Б╨╕╨│╨╜╨░╨╗ -2..+2 (╨╜╨░╨╖╨░╨┤/╨╜╨╛╨╗╤М/╨▓╨┐╨╡╤А╤С╨┤); ╤А╤Г╨║╨╛╤П╤В╨║╨░ ╨╜╨╡
        // ╨▓╤Б╤В╨░╨▓╨╗╨╡╨╜╨░ (SignalID2) - ╨Ы╨Ъ╨Ь ╨▓╤Б╤В╨░╨▓╨╗╤П╨╡╤В ╨╡╤С
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
        // ╨Ъ╨╛╨╝╨▒╨╕╨╜╨╕╤А╨╛╨▓╨░╨╜╨╜╤Л╨╣ ╨║╤А╨░╨╜: -1 ╨┤╨▓╨╛╨╣╨╜╨░╤П ╤В╤П╨│╨░ / 0 ╨┐╨╛╨╡╨╖╨┤╨╜╨╛╨╡ / +1 ╤Н╨║╤Б╤В╤А╨╡╨╜╨╜╨╛╨╡
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
QString VL60IOController::getControlStateText(const io_control_input_t &io_ctrl,
                                              float state) const
{
    if (io_ctrl.state_mode == "kme")
    {
        if (state < -0.9f)          return u8"╨┐╨╛╨╗╨╛╨╢╨╡╨╜╨╕╨╡: ╨С╨Т - ╨▒╤Л╤Б╤В╤А╨╛╨╡ ╨▓╤Л╨║╨╗╤О╤З╨╡╨╜╨╕╨╡";
        else if (state < -0.1f)     return u8"╨┐╨╛╨╗╨╛╨╢╨╡╨╜╨╕╨╡: ╨Э╨╛╨╗╤М";
        else if (state < 0.1f)      return u8"╨┐╨╛╨╗╨╛╨╢╨╡╨╜╨╕╨╡: ╨Р╨Т - ╨░╨▓╤В╨╛╨╝╨░╤В╨╕╤З╨╡╤Б╨║╨╛╨╡ ╨▓╤Л╨║╨╗╤О╤З╨╡╨╜╨╕╨╡";
        else if (state < 0.3f)      return u8"╨┐╨╛╨╗╨╛╨╢╨╡╨╜╨╕╨╡: ╨а╨Т - ╤А╤Г╤З╨╜╨╛╨╡ ╨▓╤Л╨║╨╗╤О╤З╨╡╨╜╨╕╨╡";
        else if (state < 0.5f)      return u8"╨┐╨╛╨╗╨╛╨╢╨╡╨╜╨╕╨╡: ╨д╨Т - ╤Д╨╕╨║╤Б╨░╤Ж╨╕╤П ╨▓╤Л╨║╨╗╤О╤З╨╡╨╜╨╕╤П";
        else if (state < 0.7f)      return u8"╨┐╨╛╨╗╨╛╨╢╨╡╨╜╨╕╨╡: ╨д╨Я - ╤Д╨╕╨║╤Б╨░╤Ж╨╕╤П ╨┐╤Г╤Б╨║╨░";
        else if (state < 0.9f)      return u8"╨┐╨╛╨╗╨╛╨╢╨╡╨╜╨╕╨╡: ╨а╨Я - ╤А╤Г╤З╨╜╨╛╨╣ ╨┐╤Г╤Б╨║";
        else if (state < 1.1f)      return u8"╨┐╨╛╨╗╨╛╨╢╨╡╨╜╨╕╨╡: ╨Р╨Я - ╨░╨▓╤В╨╛╨╝╨░╤В╨╕╤З╨╡╤Б╨║╨╕╨╣ ╨┐╤Г╤Б╨║";
        else
        {
            const int pos = static_cast<int>(state * 5.0f - 5.0f + 0.5f);
            return u8"╨┐╨╛╨╗╨╛╨╢╨╡╨╜╨╕╨╡: ╨┐╨╛╨╖╨╕╤Ж╨╕╤П " + QString::number(std::max(pos, 1));
        }
    }

    if (io_ctrl.type == "Crane254")
    {
        if (state < -0.01f)
        {
            return u8"╤Ж╨╡╨╗╨╡╨▓╨╛╨╡: ╨╛╤В╨┐╤Г╤Б╨║╨╜╨╛╨╡ (╨▓╤Л╨┐╤Г╤Б╨║ ╨в╨ж)";
        }

        const float p_target = state * 0.4f * 10.2f;

        return u8"╤Ж╨╡╨╗╨╡╨▓╨╛╨╡: " + QString::number(p_target, 'f', 1) + u8" ╨║╨│╤Б/╤Б╨╝┬▓";
    }

    return IOController::getControlStateText(io_ctrl, state);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GET_MODULE(VL60IOController)
