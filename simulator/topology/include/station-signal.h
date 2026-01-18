#ifndef     STATION_SIGNAL_H
#define     STATION_SIGNAL_H

#include    "train-signal.h"
#include    <combine-relay.h>

class Trajectory;
class Switch;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT StationSignal : public TrainSignal
{
    Q_OBJECT

public:

    StationSignal(QObject* parent = nullptr);

    virtual ~StationSignal();

    /// Шаг симуляции
    virtual void step(double t, double dt) override;

public slots:

    void slotPressOpenTrain();

    void slotPressOpenShunting();

    void slotPressOpenCall();

    void slotPressClose();

private slots:

    void slotOpenTimer();

    void slotCloseTimer();

    void slotBlinkTimer();

protected:

    /// Целевая траектория маневрового маршрута
    Trajectory* ref_trajectory_shunt = nullptr;

    enum
    {
        CALL_OPENED = 0,
        CALL_CLOSED,
        CALL_SELF_CTRL,
        CALL_TRAIN_CTRL,
        CALL_SHUNT_CTRL,
        NUM_CALL_CONTACTS
    };
    /// Реле пригласительного сигнала:
    /// управляется кнопками открыть пригласительный/закрыть
    Relay* call_relay = new Relay(NUM_CALL_CONTACTS);

    enum
    {
        CR_ALLOW_ROUTE = 0,
        CR_PROHIBITED_ROUTE,
        CR_SIGNAL_RELAY_CTRL,
        NUM_CR_CONTACTS
    };
    /// Контрольное маршрутное реле:
    /// включено, когда до следующего поездного светофора свободно и стрелки по маршруту
    Relay* control_relay = new Relay(NUM_CR_CONTACTS);

    enum
    {
        SR_OPENED = 0,
        SR_CLOSED,
        SR_SELF_CTRL,
        SR_CALL_CTRL,
        SR_SHUNT_CTRL,
        SR_LOCK_RELAY_CTRL,
        NUM_SR_CONTACTS,
    };
    /// Сигнальное реле:
    /// управляется кнопками открыть поездной/закрыть сигнал (если маршрут возможен)
    Relay* signal_relay = new Relay(NUM_SR_CONTACTS);

    enum
    {
        CRS_ALLOW_ROUTE = 0,
        CRS_PROHIBITED_ROUTE,
        CRS_SIGNAL_RELAY_CTRL,
        NUM_CRS_CONTACTS
    };
    /// Контрольное реле маневрового маршрута:
    /// включено, когда до целевой траектории свободно и стрелки по маршруту,
    /// либо пока светофор открыт и заняты оба участка перед и за светофором (пока проезжает состав)
    Relay* control_relay_shunt = new Relay(NUM_CRS_CONTACTS);

    enum
    {
        SRS_OPENED = 0,
        SRS_CLOSED,
        SRS_SELF_CTRL,
        SRS_CALL_CTRL,
        SRS_TRAIN_CTRL,
        SRS_LOCK_ROUTE_CTRL,
        SRS_LOCK_RELAY_CTRL,
        NUM_SRS_CONTACTS,
    };
    /// Сигнальное реле маневрового маршрута:
    /// управляется кнопками открыть маневровый/закрыть сигнал (если маршрут возможен)
    Relay* signal_relay_shunt = new Relay(NUM_SRS_CONTACTS);

    enum
    {
        LR_NEUTRAL_ROUTE_LOCKED = 0,
        LR_NEUTRAL_NO_ROUTE,
        NUM_LR_NEUTRAL_CONTACTS,

        LR_PLUS_TRAIN_LOCKED = 0,
        NUM_LR_PLUS_CONTACTS,

        LR_MINUS_SHUNT_LOCKED = 0,
        NUM_LR_MINUS_CONTACTS,
    };
    /// Реле замыкания маршрута (с полярным якорем):
    /// при питании положительным напряжением блокирует стрелки и занимает пути
    /// по поездному маршруту, при питании отрицательным напряжением - по маневровому
    CombineRelay* lock_relay = new CombineRelay(NUM_LR_NEUTRAL_CONTACTS, NUM_LR_PLUS_CONTACTS, NUM_LR_MINUS_CONTACTS);


    enum Restrict : std::uint8_t {
        SWITCHES_SIDE = 0,  ///< Маршрут с отклонением по стрелочным переводам
        SWITCHES_SIDE_80,   ///< //TODO// Маршрут с отклонением по пологим стрелочным переводам не более 1/18 (не менее 80 км/ч)
        SWITCHES_SIDE_120,  ///< //TODO// Маршрут с отклонением по пологим стрелочным переводам не более 1/22 (не менее 120 км/ч)
        SWITCHES_STRAIGHT,  ///< Маршрут без отклонений по стрелочным переводам
    };

    /// Признак ограничения по положениям стрелочных переводов
    Restrict switches_state = SWITCHES_STRAIGHT;

    /// Контакт мигания
    bool blink_contact = true;

private:

    /// Контроль маневрового маршрута: до целевой траектории или следующего светофора
    /// свободно и стрелки по маршруту
    bool is_shunt_route = false;

    /// Контроль защиты от размыкания маршрута, пока светофор открыт
    /// и заняты оба участка перед и за светофором (пока проезжает состав)
    bool is_lock_route = true;

    /// Признак нажатия кнопки открытия
    bool is_open_train_button_pressed = false;

    /// Признак нажатия кнопки открытия
    bool is_open_shunt_button_pressed = false;

    /// Признак нажатия кнопки открытия
    bool is_open_call_button_pressed = false;

    /// Признак НЕнажатия кнопки закрытия (нормально замкнутая)
    bool is_close_button_unpressed = true;

    /// Таймер выдержки времени удержания кнопки открыть
    Timer* open_timer = new Timer(1.0, false);

    /// Таймер выдержки времени удержания кнопки закрыть
    Timer* close_timer = new Timer(1.0, false);

    /// Таймер мигания линз светофора
    Timer* blink_timer = new Timer(0.75, false);

    /// Замыкание или размыкание стрелочного перевода вперёд
    bool check_and_lock_switch_fwd(Switch* sw, bool lock);

    /// Замыкание или размыкание стрелочного перевода назад
    bool check_and_lock_switch_bwd(Switch* sw, bool lock);

    /// Проверка состояния стрелок и занятости по маршруту до следующего поездного светофора
    void check_train_route();
};

#endif // STATION_SIGNAL_H
