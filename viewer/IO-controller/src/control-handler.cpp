#include    <control-handler.h>
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
void ControlHandler::sendControlSignal(const io_control_input_t &input)
{
    /*auto current = (*ctrl_inputs)[input.cabine_idx].getByKey1(input.id);

    if (current)
    {
        current->value = input.value;
        (*ctrl_inputs)[input.cabine_idx].updateByKey1(input.id, *current);
    }*/

    (*ctrl_inputs)[input.cabine_idx].updateByKey1(input.id, input);
    emit sigSendControlCommand(input.serialize());
}
