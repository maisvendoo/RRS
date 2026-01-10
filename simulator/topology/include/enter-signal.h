#ifndef     ENTER_SIGNAL_H
#define     ENTER_SIGNAL_H

#include    <rail-signal.h>
#include    <combine-relay.h>
#include    <timer.h>
#include    "trigger.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT EnterSignal : public Signal
{
public:

    EnterSignal(QObject *parent = nullptr);

    ~EnterSignal();

    void step(double t, double dt) override;

public slots:

    void slotPressOpen();

    void slotPressClose();

private:

    enum
    {
        NUM_RCR_CONTACTS = 3,
        NUM_SR_CONTACTS = 5,
        NUM_ALR_CONTACTS = 1,
        NUM_MSR_CONTACTS = 3,
        NUM_SSR_CONTACTS = 5,
        NUM_DSR_CONTACTS = 3,
        NUM_BLINK_CONTACTS = 2,

        RCR_SR_CTRL = 0,
        RCR_MSR_SSR_CTRL = 1,
        RCR_DSR_CTRL = 2,

        SR_SELF_LOCK = 0,
        SR_MSR_SSR_CTRL = 1,
        SR_ALR_CTRL = 2,
        SR_PLUS = 3,
        SR_MINUS = 4,

        ALR_MSR_SSR_CTRL = 0,

        MSR_RED = 0,
        MSR_YELLOW = 1,
        MSR_BLINK = 2,

        SSR_RED = 0,
        SSR_TOP_YELLOW = 1,
        SSR_BOTTOM_YELLOW = 2,
        SSR_SIDE = 3,
        SSR_BLINK = 4,

        DSR_TOP_YELLOW = 0,
        DSR_GREEN = 1,
        DSR_BLINK = 2,

        BLINK_GREEN = 0,
        BLINK_YELLOW = 1
    };

    /// Контрольное маршрутное реле:
    /// включено, когда до следующего светофора свободно и стрелки по маршруту
    Relay *route_control_relay = new Relay(NUM_RCR_CONTACTS);

    /// Сигнальное реле:
    /// управляется кнопками открыть/закрыть сигнал (если маршрут возможен)
    Relay *signal_relay = new Relay(NUM_SR_CONTACTS);

    /// Реле замыкания маршрута прибытия:
    /// повторяет сигнальное реле, в будущем должно блокировать стрелки от перевода
    Relay *arrival_lock_relay = new Relay(NUM_ALR_CONTACTS);

    /// Главное сигнальное реле:
    /// включено, когда сигнал открыт на маршрут прямо
    Relay *main_signal_relay = new Relay(NUM_MSR_CONTACTS);

    /// Боковое сигнальное реле:
    /// включено, когда сигнал открыт на маршрут с отклонением по стрелкам
    Relay *side_signal_relay = new Relay(NUM_SSR_CONTACTS);

    /// Сигнальное реле сквозного пропуска:
    /// включено, когда следующий светофор открыт
    Relay *direct_signal_relay = new Relay(NUM_DSR_CONTACTS);

    /// Реле мигания верхнего желтого
    Relay *blink_relay = new Relay(NUM_BLINK_CONTACTS);

    /// Признак нажатия кнопки открытия
    bool is_open_button_pressed = false;

    /// Признак НЕнажатия кнопки закрытия (нормально замкнутая)
    bool is_close_button_nopressed = true;

    /// Контакт мигания
    bool blink_contact = true;

    bool is_yellow_wire_ON = false;

    /// Напряжение путевой батареи
    double U_bat = 12.0;

    /// Таймер выдержкм времени удержания кнопки открыть
    Timer *open_timer = new Timer(1.0, false);

    /// Таймер выдержки времени удержания кнопки закрыть
    Timer *close_timer = new Timer(1.0, false);

    /// Таймер мигания верхнего желтого сигнала
    Timer *blink_timer = new Timer(0.75, false);

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override;

    /// Проверка состояния стрелок и занятости по маршруту до следующего светофора
    void check_route(bool& is_switches_side);

    /// Управление цепями питания реле
    void relay_control(bool is_switches_side);

    /// Управление миганием желтого (на предвходном)
    void yellow_blink_control();

    /// Управление состоянием линз
    void lens_control();

    /// Управление состоянием линий АЛСН
    void alsn_control();

private slots:

    void slotOpenTimer();

    void slotCloseTimer();

    void slotOnBlinkTimer();
};

#endif
