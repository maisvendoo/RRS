#ifndef     SHUNTING_SIGNAL_H
#define     SHUNTING_SIGNAL_H

#include    "rail-signal.h"

#include    <relay.h>
#include    <timer.h>

class Trajectory;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT ShuntingSignal : public Signal
{
    Q_OBJECT

public:

    ShuntingSignal(QObject *parent = nullptr);

    virtual ~ShuntingSignal();

    /// Шаг симуляции
    virtual void step(double t, double dt) override;

    void setRefTrajectory(Trajectory* trajectory)
    {
        ref_trajectory = trajectory;
    }
    Trajectory* getRefTrajectory() const
    {
        return ref_trajectory;
    }

public slots:

    void slotPressOpen();

    void slotPressClose();

private slots:

    void slotOpenTimer();

    void slotCloseTimer();

private:

    /// Целевая траектория маневрового маршрута
    Trajectory* ref_trajectory = nullptr;

    enum
    {
        CR_ALLOW_ROUTE = 0,
        CR_PROHIBITED_ROUTE,
        CR_SIGNAL_RELAY_CTRL,
        NUM_CR_CONTACTS
    };
    /// Контрольное маршрутное реле:
    /// включено, когда до целевой траектории свободно и стрелки по маршруту
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
/*TODO
    enum
    {
        UR_ROUTE_LOCKED = 0,
        UR_NO_ROUTE,
        NUM_UR_CONTACTS,
    };
    /// Реле размыкания маршрута:
    /// включается при проезде светофора всем составом и освобождении блок-участка
    Relay *unlock_relay = new Relay(NUM_LR_CONTACTS);
*/
    /// Напряжение батареи
    double U_bat = 12.0;

    /// Напряжение для контрольного маршрутного реле
    double U_ctrl = 0.0;

    /// Признак нажатия кнопки открытия
    bool is_open_button_pressed = false;

    /// Признак НЕнажатия кнопки закрытия (нормально замкнутая)
    bool is_close_button_unpressed = true;

    /// Таймер выдержки времени удержания кнопки открыть
    Timer *open_timer = new Timer(1.0, false);

    /// Таймер выдержки времени удержания кнопки закрыть
    Timer *close_timer = new Timer(1.0, false);

    /// Проверка состояния стрелок и занятости по маршруту
    void check_shunt_route();
};

#endif // SHUNTING_SIGNAL_H
