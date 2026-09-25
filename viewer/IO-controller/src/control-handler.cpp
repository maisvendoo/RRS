#include    <control-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ControlHandler::load_config(CfgReader &cfg, QDomNode secNode)
{
    cfg.getString(secNode, "Name", name);
    cfg.getString(secNode, "Description", description);

    int control_ID = 0;
    cfg.getInt(secNode, "ID", control_ID);
    id = static_cast<uint16_t>(control_ID);

    QString keyName = "";
    cfg.getString(secNode, "KeyName", keyName);
    keyCode = KeySymbolsRRSMap.value(keyName, KEY_Undefined);

    cfg.getString(secNode, "KeyModOnName", keyModOnName);

    cfg.getString(secNode, "KeyModOffName", keyModOffName);

    if (keyModOffName.isEmpty())
    {
        keyModOffName = keyModOnName;
    }

    if (!keyName.isEmpty())
    {
        hot_keys = "Клавиши: ";

        if (!keyModOnName.isEmpty())
        {
            hot_keys += keyModOnName + "+" + keyName.mid(4);
        }

        if (!keyModOffName.isEmpty() && keyModOnName != keyModOffName)
        {
            hot_keys += " | " + keyModOffName + "+" + keyName.mid(4);
        }
    }
    else
    {
        hot_keys = QString();
    }

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
void ControlHandler::sendControlSignal()
{
    emit sigSendControlCommand(serialize());
}
