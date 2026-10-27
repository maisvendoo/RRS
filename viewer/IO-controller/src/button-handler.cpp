#include    <button-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ButtonHandler::ButtonHandler(QObject *parent) : ControlHandler(parent)
{

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
            new_value = 1.0f;
        }
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
            if (is_pressed && button == CTRL_RIGHT_MOUSE_BUTTON)
            {
                is_holded = true;
                new_value = 1.0f;
                setValueIfChanged(new_value);
            }
        }
    }

    if (is_pressed && button == CTRL_LEFT_MOUSE_BUTTON)
    {
        is_holded = false;
        new_value = 1.0f;
        setValueIfChanged(new_value);
    }
    else if (!is_pressed && button == CTRL_LEFT_MOUSE_BUTTON)
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
    QString usage_msg = QString("Нажать: ЛКМ | Отпустить: ЛКМ");

    if (allow_hold)
    {
        usage_msg += " | Удерживать: ПКМ";
    }

    return usage_msg;
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString ButtonHandler::getState() const
{
    bool state = static_cast<bool>(getSignalValue());

    QString press_msg = QString("Нажато");

    if (is_holded)
    {
        press_msg += " и удерживается";
    }

    return state ? press_msg : QString("Отпущено");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ButtonHandler::load_config(CfgReader &cfg, QDomNode secNode)
{
    ControlHandler::load_config(cfg, secNode);

    cfg.getBool(secNode, "AllowedHold", allow_hold);

    return true;
}

