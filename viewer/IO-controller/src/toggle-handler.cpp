#include    <toggle-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ToggleHandler::ToggleHandler(QObject *parent) : ControlHandler(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ToggleHandler::processKeyInput(const std::set<uint16_t> &pressed_keys)
{
    if (getKeyState(pressed_keys, keyCode))
    {
        if (keyModOnName == keyModOffName)
        {
            if (isKeyModifier(pressed_keys, keyModOnName))
            {
                setValueIfChanged(1.0f - value);
                return;
            }
        }

        if (isKeyModifier(pressed_keys, keyModOnName))
        {
            setValueIfChanged(1.0f);
            return;
        }

        if (isKeyModifier(pressed_keys, keyModOffName))
        {
            setValueIfChanged(0.0f);
            return;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ToggleHandler::processMouseInput(uint32_t button, bool is_pressed)
{
    if (!is_pressed)
    {
        return;
    }

    if (button == getButtonCode(onButton) && !toBool())
    {
        setValueIfChanged(1.0f);
        return;
    }

    if (button == getButtonCode(offButton) && toBool())
    {
        setValueIfChanged(0.0f);
        return;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString ToggleHandler::getUsage() const
{
    QString onButtonName = getButtonTooltip(getButtonCode(onButton));
    QString offButtonName = getButtonTooltip(getButtonCode(offButton));

    return QString("Вкл.: %1 | Выкл.: %2").arg(onButtonName).arg(offButtonName);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString ToggleHandler::getState() const
{
    bool state = static_cast<bool>(getSignalValue());

    return state ? QString("ВКЛ") : QString("ВЫКЛ");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ToggleHandler::load_config(CfgReader &cfg, QDomNode secNode)
{
    cfg.getString(secNode, "OnButton", onButton);
    cfg.getString(secNode, "OffButton", offButton);

    return true;
}


