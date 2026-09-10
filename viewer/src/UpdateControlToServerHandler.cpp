#include "UpdateControlToServerHandler.h"

#include <vsg/ui/ApplicationEvent.h>

#include <Logger.h>
//#include "Logger.h"
#include "tcp-client.h"
#include "key-symbols.h"
#include "controlled-struct.h"
#include <cstddef>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
UpdateControlToServerHandler::UpdateControlToServerHandler(TcpClient* tc)
    : _tcp_client(tc)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
namespace
{

/// Сервисные клавиши деповского питания (ТЗ "Деповское питание"):
/// проходят на сервер даже в пешом режиме (подключение кабеля игроком
/// у розетки ПЕ: K - кабель, L - питание колонки, O - вводной аппарат)
const std::set<std::uint16_t>& serviceKeys()
{
    static const std::set<std::uint16_t> keys = {KEY_K, KEY_L, KEY_O};
    return keys;
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateControlToServerHandler::apply(vsg::KeyPressEvent& keyPress)
{
//    LOG_INFO("KeyProbe press %u", keyPress.keyBase);
    // Пешая ходьба: клавиши управляют игроком, на сервер не уходят
    // (кроме сервисных клавиш деповского питания)
    if (_is_control_suppressed && !serviceKeys().count(keyPress.keyBase))
    {
        return;
    }

    // Массив нажатых клавиш для сервера
    if (KeySymbolsRRS.count(keyPress.keyBase))
    {
        auto result = _pressed_keys.insert(keyPress.keyBase);
        if (result.second)
        {
            sendControlToServer();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateControlToServerHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
//    LOG_INFO("release %u", keyRelease.keyBase);
    if (_is_control_suppressed && !serviceKeys().count(keyRelease.keyBase))
    {
        return;
    }

    if (_pressed_keys.erase(keyRelease.keyBase))
    {
        sendControlToServer();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateControlToServerHandler::changeCurrentVehicle(int current_idx, int controlled_idx, int cabine_idx)
{
    if ((current_idx < 0) || (controlled_idx < 0))
    {
        return;
    }

    _current_idx = current_idx;
    _controlled_idx = controlled_idx;
    _controlled_cabine_idx = cabine_idx;
    sendControlToServer();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateControlToServerHandler::setNeedDebugMsg(bool is_needed)
{
    _is_needed_debug_msg = is_needed;
    sendControlToServer();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateControlToServerHandler::setSpeedFactor(int speed_factor)
{
    _tcp_client->sendSimSpeedCommand(speed_factor);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateControlToServerHandler::setControlSuppressed(bool suppressed)
{
    if (_is_control_suppressed == suppressed)
        return;

    _is_control_suppressed = suppressed;

    // Сброс накопленных нажатий: поезд не должен "держать клавиши",
    // зажатые до входа в пешей режим
    _pressed_keys.clear();
    sendEmptyControlToServer();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateControlToServerHandler::apply([[maybe_unused]] vsg::FocusInEvent& focusIn)
{
    sendControlToServer();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateControlToServerHandler::apply([[maybe_unused]] vsg::FocusOutEvent& focusOut)
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
    constexpr KeySymbol modifier_keys[] = {KEY_Shift_L, KEY_Shift_R, KEY_Control_L, KEY_Control_R, KEY_Alt_L, KEY_Alt_R};
    std::size_t modifiers_size = 0;
    for (std::uint16_t key : modifier_keys)
    {
        if (_pressed_keys.count(key))
        {
            ++modifiers_size;
        }
    }

    if (_pressed_keys.size() == modifiers_size)
    {
        sendEmptyControlToServer();
        return;
    }

    controlled_t controlled;
    controlled.current_vehicle = _current_idx;
    controlled.controlled_vehicle = _controlled_idx;
    controlled.controlled_cabine_idx = _controlled_cabine_idx;
    controlled.need_debug_msg = _is_needed_debug_msg;

    // Alt в момент клика по органу - режим подсказок клиента, он не
    // должен попадать в набор (тумблеры различают MODIFIER_OnlyAlt)
    const bool alt_suppressed = _inject_suppress_alt && !_injected_keys.empty();

    // Отправляем массив управляющих клавиш
    for (auto key : _pressed_keys)
    {
        if (alt_suppressed && ((key == KEY_Alt_L) || (key == KEY_Alt_R)))
        {
            continue;
        }

        // F-клавиши не отправляем без модификаторов Shift, Ctrl или Alt
        if ((key >= KEY_F1) && (key <= KEY_F12) && (modifiers_size == 0))
        {
            continue;
        }
        controlled.pressed_keys.push_back(key);
    }

    _tcp_client->sendVehicleControl(controlled.serialize());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateControlToServerHandler::sendEmptyControlToServer()
{
    // Отправляем пустой пустой массив управляющих клавиш
    controlled_t controlled;
    controlled.current_vehicle = _current_idx;
    controlled.controlled_vehicle = _controlled_idx;
    controlled.controlled_cabine_idx = _controlled_cabine_idx;
    controlled.need_debug_msg = _is_needed_debug_msg;
    controlled.pressed_keys.clear();

    _tcp_client->sendVehicleControl(controlled.serialize());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateControlToServerHandler::injectKeys(const std::vector<std::uint16_t>& keys,
                                              int duration_ms,
                                              bool suppress_alt)
{
    if (keys.empty())
        return;

    // Перекрытие кликов (быстрое ЛКМ после ПКМ и наоборот): прежний
    // инжект снимается МГНОВЕННО, иначе сервер видит W+S / A+D
    // одновременно и рычаги гасят друг друга
    if (!_injected_keys.empty())
    {
        for (auto key : _injected_keys)
        {
            _pressed_keys.erase(key);
        }

        _injected_keys.clear();
    }

    for (auto key : keys)
    {
        if (key == 0)
            continue;

        _injected_keys.push_back(key);

        if (_pressed_keys.insert(key).second)
        {
            // новое нажатие
        }
    }

    _injected_duration = duration_ms / 1000.0;
    _injected_until = -1.0;   // якорем станет первый кадр (FrameEvent)
    _inject_suppress_alt = suppress_alt;

    sendControlToServer();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateControlToServerHandler::apply(vsg::FrameEvent& frame)
{
    // Переотправка удержанных клавиш раз в 0.5 с: серверная сторона
    // кранов непрерывна (пока нажато - ручка идёт), а разовые пустые
    // пакеты (фокус-события, подтверждения Enter) могли сбивать
    // удержание
    if (!_pressed_keys.empty() && !_is_control_suppressed)
    {
        const double t = frame.frameStamp->simulationTime;

        if (t - _last_resend_time >= 0.5)
        {
            _last_resend_time = t;
            sendControlToServer();
        }
    }

    // Снятие программного нажатия органа кабины: клавиши держались
    // дольше инжекта - отпускаем (клик != удержание)
    if (!_injected_keys.empty())
    {
        const double t = frame.frameStamp->simulationTime;

        if (_injected_until < 0.0)
        {
            _injected_until = t + _injected_duration;
        }
        else if (t >= _injected_until)
        {
            for (auto key : _injected_keys)
            {
                _pressed_keys.erase(key);
            }

            _injected_keys.clear();
            _inject_suppress_alt = false;
            sendControlToServer();
        }
    }
}
