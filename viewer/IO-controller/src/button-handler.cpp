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

    if (is_pressed && button == CTRL_LEFT_MOUSE_BUTTON)
    {
        new_value = 1.0f;
        setValueIfChanged(new_value);
    }
    else if (!is_pressed && button == CTRL_LEFT_MOUSE_BUTTON)
    {
        new_value = 0.0f;
        setValueIfChanged(new_value);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString ButtonHandler::getUsage() const
{
    return QString("Нажать: ЛКМ | Отпустить: ЛКМ");
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString ButtonHandler::getState() const
{
    bool state = static_cast<bool>(getSignalValue());

    return state ? QString("Нажато") : QString("Отпущено");
}

