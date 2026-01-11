#ifndef     STATION_SIGNAL_H
#define     STATION_SIGNAL_H

#include    "train-signal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT StationSignal : public TrainSignal
{
    Q_OBJECT

public:

    StationSignal(QObject *parent = nullptr);

    virtual ~StationSignal();

    /// Шаг симуляции
    virtual void step(double t, double dt) override;

public slots:

    void slotPressOpen();

    void slotPressClose();

protected:

    enum
    {
        CR_ALLOW_ROUTE = 0,
        CR_PROHIBITED_ROUTE,
        CR_SIGNAL_RELAY_CTRL,
        NUM_CR_CONTACTS
    };
    /// Контрольное маршрутное реле:
    /// включено, когда до следующего светофора свободно и стрелки по маршруту
    Relay *control_relay = new Relay(NUM_CR_CONTACTS);

    enum
    {
        SR_OPENED = 0,
        SR_CLOSED,
        SR_SELF_CTRL,
        SR_LOCK_RELAY_CTRL,
        NUM_SR_CONTACTS,
    };
    /// Сигнальное реле:
    /// управляется кнопками открыть/закрыть сигнал (если маршрут возможен)
    Relay *signal_relay = new Relay(NUM_SR_CONTACTS);

    enum
    {
        LR_ROUTE_LOCKED = 0,
        LR_NO_ROUTE,
        NUM_LR_CONTACTS,
    };
    /// Реле замыкания маршрута:
    /// при открытии сигнала отключается и блокирует стрелки по маршруту от перевода
    Relay *lock_relay = new Relay(NUM_LR_CONTACTS);

private:

    /// Признак нажатия кнопки открытия
    bool is_open_button_pressed = false;

    /// Признак НЕнажатия кнопки закрытия (нормально замкнутая)
    bool is_close_button_unpressed = true;

    /// Таймер выдержки времени удержания кнопки открыть
    Timer *open_timer = new Timer(1.0, false);

    /// Таймер выдержки времени удержания кнопки закрыть
    Timer *close_timer = new Timer(1.0, false);

private slots:

    void slotOpenTimer();

    void slotCloseTimer();
};

#endif // STATION_SIGNAL_H
