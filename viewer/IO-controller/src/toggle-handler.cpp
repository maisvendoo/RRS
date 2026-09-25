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
                value = 1.0f - value;
                sendControlSignal();
                return;
            }
        }

        if (isKeyModifier(pressed_keys, keyModOnName))
        {
            value = 1.0f;
            sendControlSignal();
            return;
        }

        if (isKeyModifier(pressed_keys, keyModOffName))
        {
            value = 0.0f;
            sendControlSignal();
            return;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ToggleHandler::processMouseInput(uint32_t button, bool is_pressed)
{
    if (button == CTRL_LEFT_MOUSE_BUTTON && !toBool())
    {
        value = 1.0f;
        sendControlSignal();
        return;
    }

    if (button == CTRL_RIGHT_MOUSE_BUTTON && toBool())
    {
        value = 0.0f;
        sendControlSignal();
        return;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString ToggleHandler::getUsage() const
{
    return QString("Вкл.: ЛКМ | Выкл.: ПКМ");
}


