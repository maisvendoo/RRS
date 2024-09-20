#ifndef     ENTER_SIGNAL_H
#define     ENTER_SIGNAL_H

#include    <rail-signal.h>
#include    <relay.h>
#include    <timer.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT EnterSignal : public Signal
{
public:

    EnterSignal(QObject *parent = Q_NULLPTR);

    ~EnterSignal();

    void step(double t, double dt) override;

private:

    enum
    {
        NUM_MSR_CONTACTS = 2,
        NUM_SSR_CONTACTS = 3,
        NUM_DSR_CONTACTS = 2,
        NUM_RCR_CONTACTS = 3,
        NUM_SR_CONTACTS = 3,
        NUM_ALR_CONTACTS = 1,
        NUM_ESR_CONTACTS = 1,

        MSR_RED = 0,
        MSR_YELLOW = 1,

        SSR_RED = 0,
        SSR_TOP_YELLOW = 1,
        SSR_BOTTOM_YELLOW = 2,

        DSR_TOP_YELLOW = 0,
        DSR_GREEN = 1
    };

    /// Главное сигнальное реле
    Relay *main_signal_relay = new Relay(NUM_MSR_CONTACTS);

    /// Боковое сигнальное реле
    Relay *side_signal_relay = new Relay(NUM_SSR_CONTACTS);

    /// Сигнальное реле сквозного пропуска
    Relay *direct_signal_relay = new Relay(NUM_DSR_CONTACTS);

    /// Контрольное маршрутное реле
    Relay *route_control_relay = new Relay(NUM_RCR_CONTACTS);

    /// Сигнальное реле
    Relay *signal_relay = new Relay(NUM_SR_CONTACTS);

    /// Реле замыкания маршрута прибытия
    Relay *arrival_lock_relay = new Relay(NUM_ALR_CONTACTS);

    /// Указательное реле выходного светофора
    Relay *exit_signal_relay = new Relay(NUM_ESR_CONTACTS);

    /// Признак нажатия кнопки открытия
    bool is_open_button_pressed = false;

    /// Признак НЕнажатия кнопки закрытия (нормально замкнутая)
    bool is_close_button_nopressed = true;

    /// Таймер выдержкм времени удержания кнопки открыть
    Timer *open_timer = new Timer(1.0);

    /// Таймер выдержки времени удержания кнопки закрыть
    Timer *close_timer = new Timer(1.0);

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override;

private slots:

    void slotOpenTimer();

    void slotCloseTimer();

    void slotPressOpen();

    void slotPressClose();
};

#endif
