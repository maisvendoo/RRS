#pragma once
#ifndef UPDATE_CONTROL_TO_SERVER_HANDLER_H
#define UPDATE_CONTROL_TO_SERVER_HANDLER_H

#include <vsg/ui/KeyEvent.h>
#include <cstdint>
#include <set>

class TcpClient;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class UpdateControlToServerHandler final : public vsg::Inherit<vsg::Visitor, UpdateControlToServerHandler>
{
public:
    explicit UpdateControlToServerHandler(TcpClient *tc);

    void apply(vsg::KeyPressEvent& keyPress) override;
    void apply(vsg::KeyReleaseEvent& keyRelease) override;
    void apply(vsg::FocusInEvent& focusIn) override;
    void apply(vsg::FocusOutEvent& focusOut) override;
    void apply(vsg::FrameEvent& frame) override;
    void changeCurrentVehicle(int current_idx, int controlled_idx, int cabine_idx);
    void setNeedDebugMsg(bool is_needed);

    void setSpeedFactor(int speed_factor);

    /// Подавить пересылку управляющих клавиш на сервер (пешая ходьба,
    /// ТЗ "walking": WASD/Space/E управляют игроком, а не поездом).
    /// Включение сбрасывает накопленные нажатия и шлёт пустое управление
    void setControlSuppressed(bool suppressed);

    /// Программное нажатие клавиш органа кабины (клик мышью по Alt,
    /// ТЗ "Взаимодействие с элементами кабины"): клавиши добавляются
    /// к отправляемому набору на duration_ms и снимаются автоматически.
    /// suppress_alt - убрать Alt из посылаемого набора на время нажатия
    /// (тумблеры различают модификаторы Only-*)
    void injectKeys(const std::vector<std::uint16_t>& keys,
                    int duration_ms,
                    bool suppress_alt);

private:

    double _last_resend_time = 0.0;

    void sendControlToServer();
    void sendEmptyControlToServer();

    TcpClient *_tcp_client = nullptr;

    std::uint16_t _current_idx = 0;
    std::uint16_t _controlled_idx = 0;
    std::uint16_t _controlled_cabine_idx = 0;
    bool _is_needed_debug_msg = false;
    bool _is_control_suppressed = false;
    std::set<std::uint16_t> _pressed_keys = {};

    /// Активное программное нажатие (клик по органу кабины).
    /// _injected_until < 0 - якорение на ближайшем кадре (duration
    /// в _injected_duration), затем абсолютное время снятия
    std::vector<std::uint16_t> _injected_keys = {};
    double _injected_until = 0.0;
    double _injected_duration = 0.0;
    bool _inject_suppress_alt = false;
};

#endif // UPDATE_CONTROL_TO_SERVER_HANDLER_H
