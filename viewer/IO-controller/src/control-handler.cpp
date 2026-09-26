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
float ControlHandler::getSignalValue() const
{
    if (!animation_signals_map)
    {
        return 0.0f;
    }

    if (contolledObjectName.isEmpty() || feedback_signals == nullptr)
    {
        return 0.0f;
    }

    auto it = animation_signals_map->find(contolledObjectName);

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
bool ControlHandler::isKeyModifier(const std::set<uint16_t> &keys,
                                   const QString &modName)
{
    // Имя модификатора пусто,
    if (modName.isEmpty())
    {
        // ну ок, но тогда ни один не должен быть нажат при этой проверке!
        return !isAnyModifier(keys);
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
bool ControlHandler::isAnyModifier(const std::set<uint16_t> &keys) const
{
    return isShift(keys) || isControl(keys) || isAlt(keys);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControlHandler::sendControlSignal()
{
    emit sigSendControlCommand(serialize());
}
