#ifndef     ENTER_SIGNAL_H
#define     ENTER_SIGNAL_H

#include    <rail-signal.h>
#include    <combine-releay.h>
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

    void setFwdBusy(bool is_fwd_busy)
    {
        this->is_fwd_busy = is_fwd_busy;
    }

    void setBwdBusy(bool is_bwd_busy)
    {
        this->is_bwd_busy = is_bwd_busy;
    }

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
        DSR_GREEN = 1,

        RCR_SR_CTRL = 0,
        RCR_MSR_SSR_CTRL = 1,
        RCR_DSR_CTRL = 2,

        SR_SELF_LOCK = 0,
        SR_MSR_SSR_CTRL = 1,
        SR_ALR_CTRL = 2,

        ALR_MSR_SSR_CTRL = 0,

        ESR_DSR_CTRL = 0
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

    enum
    {
        NUM_FWD_BUSY = 3,
        FWD_BUSY_RED = 0,

        NUM_BWD_BUSY = 3,
        BWD_BUSY_PLUS = 0,
        BWD_BUSY_MINUS = 1,
        BWD_BUSY_CLOSE = 2
    };

    /// Путевое реле на учатке приближения
    Relay *fwd_way_relay = new Relay(NUM_FWD_BUSY);

    /// Путевое реле на стрелочном участке
    Relay *bwd_way_relay = new Relay(NUM_BWD_BUSY);

    /// Признак занятия учатка приближения
    bool is_fwd_busy = false;

    /// Признак занятия стрелочного участка
    bool is_bwd_busy = false;

    /// Признак нажатия кнопки открытия
    bool is_open_button_pressed = false;

    /// Признак НЕнажатия кнопки закрытия (нормально замкнутая)
    bool is_close_button_nopressed = true;

    double U_bat = 12.0;

    /// Напряжение, передаваемое на линию предыдущего светофора
    double U_line_prev = 0.0;

    /// Таймер выдержкм времени удержания кнопки открыть
    Timer *open_timer = new Timer(1.0);

    /// Таймер выдержки времени удержания кнопки закрыть
    Timer *close_timer = new Timer(1.0);

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override;

    /// Управление состоянием линз
    void lens_control();

    /// Контроль занятости примыкающих участков
    void busy_control();

    /// Управление цепями питания реле
    void relay_control();

    /// Проверка занятости маршрута по текущим стрелкам
    bool is_route_free(Connector *conn);

    /// Проверка состояния стрелок по маршруту
    bool is_switch_minus(Connector *conn);

private slots:

    void slotOpenTimer();

    void slotCloseTimer();

    void slotPressOpen();

    void slotPressClose();
};

#endif
