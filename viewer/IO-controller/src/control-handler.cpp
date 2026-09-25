#include    <control-handler.h>
#include    <io-controller-keymap.h>
#include    <CfgReader.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ControlHandler::load_config(CfgReader &cfg, int cabs_num)
{
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float ControlHandler::getSignalValueByName(const QString &objectName) const
{
    if (objectName.isEmpty() || feedback_signals == nullptr)
    {
        return 0.0f;
    }

    auto it = animation_signals_map->find(objectName);

    if (it != animation_signals_map->end())
    {
        uint16_t signal_id = it.value();

        if (signal_id < feedback_signals->size())
        {
            float state = (*feedback_signals)[signal_id];
            return state;
        }
    }

    return 0.0f;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float ControlHandler::getSignalValueByID(uint16_t control_id, int cab_idx) const
{
    auto ic_input = (*ctrl_inputs)[cab_idx].getByKey1(control_id);

    if (!ic_input)
    {
        return 0.0f;
    }

    return getSignalValueByName(ic_input->contolledObjectName);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ControlHandler::getKeyState(const std::set<uint16_t> &keys, uint16_t key)
{
    if (key == 0 || key == KEY_Undefined)
    {
        return false;
    }

    return keys.find(key) != keys.end();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ControlHandler::isKeyModifier(const std::set<uint16_t> &keys,
                                   const QString &modName)
{
    if (modName.isEmpty())
    {
        return true;
    }

    if (modName == "Shift")
    {
        return isShift(keys);
    }

    if (modName == "Ctrl")
    {
        return isControl(keys);
    }

    if (modName == "Alt")
    {
        return isAlt(keys);
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControlHandler::sendControlSignal(int vehicle_idx,
                                       int cab_idx,
                                       uint16_t id,
                                       float value)
{
    io_control_input_t out;
    out.controlled_vehicle_idx = vehicle_idx;
    out.cabine_idx = cab_idx;
    out.id = id;
    out.value = value;

    (*ctrl_inputs)[cab_idx].updateByKey1(id, out);
    emit sigSendControlCommand(out.serialize());
}
