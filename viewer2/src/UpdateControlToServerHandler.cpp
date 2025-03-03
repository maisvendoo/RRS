#include "UpdateControlToServerHandler.h"
#include "tcp-client.h"
#include "key-symbols.h"
#include "controlled-struct.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
UpdateControlToServerHandler::UpdateControlToServerHandler(TcpClient *tc)
    : _tcp_client(tc)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateControlToServerHandler::apply(vsg::KeyPressEvent& keyPress)
{
    // Массив нажатых клавиш для сервера
    if (KeySymolsRRS.count(keyPress.keyBase))
    {
        auto result = _pressed_keys.insert(keyPress.keyBase);
        if (result.second)
            sendControlToServer();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateControlToServerHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    if (_pressed_keys.erase(keyRelease.keyBase))
        sendControlToServer();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateControlToServerHandler::changeCurrentVehicle(int current_idx, int controlled_idx)
{
    if ((current_idx < 0) || (controlled_idx < 0))
        return;

    _current_idx = current_idx;
    _controlled_idx = controlled_idx;
    sendControlToServer();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateControlToServerHandler::apply(vsg::FocusInEvent& focusIn)
{
    sendControlToServer();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateControlToServerHandler::apply(vsg::FocusOutEvent& focusOut)
{
    sendEmptyControlToServer();
    _pressed_keys.clear();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateControlToServerHandler::sendControlToServer()
{
    // Если массив нажатых клавиш пустой
    // отправляем пустое управление
    if (_pressed_keys.empty())
    {
        sendEmptyControlToServer();
        return;
    }

    // Если массив нажатых клавиш содержит только Shift, Ctrl, Alt
    // отправляем пустое управление
    int modifiers_size = 0;
    for (uint16_t key : {KEY_Shift_L, KEY_Shift_R,
                         KEY_Control_L, KEY_Control_R,
                         KEY_Alt_L, KEY_Alt_R})
    {
        if (_pressed_keys.count(key))
            ++modifiers_size;
    }
    if (_pressed_keys.size() == modifiers_size)
    {
        sendEmptyControlToServer();
        return;
    }

    controlled_t c;
    c.current_vehicle = _current_idx;
    c.controlled_vehicle = _controlled_idx;

    // Отправляем массив управляющих клавиш
    for (auto key : _pressed_keys)
        c.pressed_keys.push_back(key);

    _tcp_client->sendVehicleControl(c.serialize());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateControlToServerHandler::sendEmptyControlToServer()
{
    // Отправляем пустой пустой массив управляющих клавиш
    controlled_t c;
    c.current_vehicle = _current_idx;
    c.controlled_vehicle = _controlled_idx;
    c.pressed_keys.clear();

    _tcp_client->sendVehicleControl(c.serialize());
}
