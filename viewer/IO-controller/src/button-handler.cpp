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
    if (getKeyState(pressed_keys, keyCode))
    {
        if (isKeyModifier(pressed_keys, keyModOnName) ||
            keyModOnName.isEmpty())
        {
            value = 1.0f;
        }
    }
    else
    {
        value = 0.0f;
    }

    sendControlSignal();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ButtonHandler::processMouseInput(uint32_t button, bool is_pressed)
{

    if (is_pressed && button == CTRL_LEFT_MOUSE_BUTTON)
    {
        value = 1.0f;
        sendControlSignal();
    }
    else if (!is_pressed && button == CTRL_LEFT_MOUSE_BUTTON)
    {
        value = 0.0f;
        sendControlSignal();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString ButtonHandler::getUsage() const
{
    return QString("Нажать: ЛКМ | Отпустить: ЛКМ");
}

