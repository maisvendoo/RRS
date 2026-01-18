#ifndef     SHUNTING_SIGNAL_H
#define     SHUNTING_SIGNAL_H

#include    "rail-signal.h"

#include    <relay.h>
#include    <timer.h>

class Trajectory;
class Switch;

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
        ref_trajectory_shunt = trajectory;
    }
    Trajectory* getRefTrajectory() const
    {
        return ref_trajectory_shunt;
    }

public slots:

    void slotPressOpenShunting();

    void slotPressClose();

private slots:

    void slotOpenTimerShunt();

    void slotCloseTimer();

private:

    /// Целевая траектория маневрового маршрута
    Trajectory* ref_trajectory_shunt = nullptr;

    enum
    {
        CRS_ALLOW_ROUTE = 0,
        CRS_PROHIBITED_ROUTE,
        CRS_SIGNAL_RELAY_CTRL,
        NUM_CRS_CONTACTS
    };
    /// Контрольное реле маневрового маршрута:
    /// включено, когда до целевой траектории свободно и стрелки по маршруту
    Relay *control_relay_shunt = new Relay(NUM_CRS_CONTACTS);

    enum
    {
        SRS_OPENED = 0,
        SRS_CLOSED,
        SRS_SELF_CTRL,
        SRS_LOCK_RELAY_CTRL,
        SRS_UNLOCK_RELAY_CTRL,
        NUM_SRS_CONTACTS,
    };
    /// Сигнальное реле:
    /// управляется кнопками открыть/закрыть сигнал (если маршрут возможен)
    Relay *signal_relay_shunt = new Relay(NUM_SRS_CONTACTS);

    enum
    {
        LRS_ROUTE_LOCKED = 0,
        LRS_NO_ROUTE,
        NUM_LRS_CONTACTS,
    };
    /// Реле замыкания маневрового маршрута:
    /// при открытии сигнала отключается и блокирует стрелки по маршруту от перевода
    Relay *lock_relay_shunt = new Relay(NUM_LRS_CONTACTS);

    enum
    {
        URS_ROUTE_LOCKED = 0,
        URS_ROUTE_UNLOCKED,
        URS_SIGNAL_RELAY_CTRL,
        NUM_URS_CONTACTS,
    };
    /// Реле размыкания маневрового маршрута:
    /// включается при освобождении участка перед или за светофором (проезде всем составом)
    Relay *unlock_relay_shunt = new Relay(NUM_URS_CONTACTS);

    /// Напряжение батареи
    double U_bat = 12.0;

    /// Напряжение для контрольного реле маневрового маршрута
    double U_ctrl_shunt = 0.0;

    /// Напряжение для реле размыкания маневрового маршрута
    double U_unlock_shunt = 0.0;

    /// Признак нажатия кнопки открытия
    bool is_open_shunt_button_pressed = false;

    /// Признак НЕнажатия кнопки закрытия (нормально замкнутая)
    bool is_close_button_unpressed = true;

    /// Таймер выдержки времени удержания кнопки открыть
    Timer *open_timer = new Timer(1.0, false);

    /// Таймер выдержки времени удержания кнопки закрыть
    Timer *close_timer = new Timer(1.0, false);

    /// Замыкание или размыкание стрелочного перевода вперёд
    bool check_and_lock_switch_fwd(Switch* sw, bool lock);

    /// Замыкание или размыкание стрелочного перевода назад
    bool check_and_lock_switch_bwd(Switch* sw, bool lock);

    /// Проверка состояния стрелок и занятости по маршруту
    void check_shunt_route();
};

#endif // SHUNTING_SIGNAL_H
