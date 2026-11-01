#include    <button-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ButtonHandler::ButtonHandler(QObject *parent) : BiStateHandler(parent)
{
    positionNames = {"Отпущено", "Нажато"};
    actionNames = {"Отпустить", "Нажать"};
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ButtonHandler::processKeyInput(const std::set<uint16_t> &pressed_keys)
{
    float new_value = 0.0f;

    if (getKeyState(pressed_keys, keyCode))
    {
        if (isKeyModifier(pressed_keys, keyModOnName) ||
            keyModOnName.isEmpty())
        {
            is_holded = false;
            new_value = 1.0f;
        }
    }

    if (allow_hold && is_holded)
    {
        return;
    }

    setValueIfChanged(new_value);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ButtonHandler::processMouseInput(uint32_t button, bool is_pressed)
{
    float new_value = 0.0f;

    // Если разрешено удержание кнопки
    if (allow_hold)
    {
        // и кнопка не удерживается в данный момент
        if (!is_holded)
        {
            // Включить и держать (ПКМ)
            if (is_pressed && button == getButtonCode(holdButton))
            {
                is_holded = true;
                new_value = 1.0f;
                setValueIfChanged(new_value);
            }
        }
    }

    if (is_pressed && button == getButtonCode(pressButton))
    {
        is_holded = false;
        new_value = 1.0f;
        setValueIfChanged(new_value);
    }
    else if (!is_pressed && button == getButtonCode(pressButton))
    {
        is_holded = false;
        new_value = 0.0f;
        setValueIfChanged(new_value);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString ButtonHandler::getUsage() const
{
    QString pressButtonName = getButtonTooltip(getButtonCode(pressButton));
    QString holdButtonName = getButtonTooltip(getButtonCode(holdButton));

    QString usage_msg = QString("%3: %1 | %4: %2")
                            .arg(pressButtonName)
                            .arg(pressButtonName)
                            .arg(getActionName(1))
                            .arg(getActionName(0));

    if (allow_hold)
    {
        usage_msg += QString(" | Удерживать: %1").arg(holdButtonName);
    }

    return usage_msg;
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString ButtonHandler::getState() const
{
    bool state = static_cast<bool>(getSignalValue());

    QString press_msg = QString(getPositionName(1));

    if (is_holded)
    {
        press_msg += " и удерживается";
    }

    return state ? press_msg : QString(getPositionName(0));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ButtonHandler::load_config(CfgReader &cfg, QDomNode secNode)
{
    BiStateHandler::load_config(cfg, secNode);

    cfg.getBool(secNode, "AllowHold", allow_hold);
    cfg.getString(secNode, "PressButton", pressButton);
    cfg.getString(secNode, "HoldButton", holdButton);

    return true;
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ButtonHandler::reset()
{
    is_holded = false;
    setValueIfChanged(0.0f);
}

