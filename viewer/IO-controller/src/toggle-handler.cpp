#include    <toggle-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ToggleHandler::ToggleHandler(QObject *parent) : BiStateHandler(parent)
{
    positionNames = {"ВЫКЛ", "ВКЛ"};
    actionNames = {"Выкл.", "Вкл."};
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

    return QString("%3: %1 | %4: %2")
        .arg(onButtonName)
        .arg(offButtonName)
        .arg(getActionName(1))
        .arg(getActionName(0));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString ToggleHandler::getState() const
{
    bool state = static_cast<bool>(getSignalValue());

    return state ? QString(getPositionName(1)) : QString(getPositionName(0));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ToggleHandler::load_config(CfgReader &cfg, QDomNode secNode)
{
    BiStateHandler::load_config(cfg, secNode);

    cfg.getString(secNode, "OnButton", onButton);
    cfg.getString(secNode, "OffButton", offButton);

    return true;
}


