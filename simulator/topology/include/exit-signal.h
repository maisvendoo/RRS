#ifndef     EXIT_SIGNAL_H
#define     EXIT_SIGNAL_H

#include    <rail-signal.h>
#include    <combine-releay.h>
#include    <timer.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT ExitSignal : public Signal
{
public:

    ExitSignal(QObject *parent = Q_NULLPTR);

    ~ExitSignal();

    void step(double t, double dt) override;

public slots:

    void slotPressOpen();

    void slotPressClose();

private:

    enum
    {
        NUM_SR_CONTACTS = 3,
        SR_SELF = 0,
        SR_DLR_CTRL = 1,
        SR_SRS_CTRL = 2
    };

    /// Сигнальное реле
    Relay *signal_relay = new Relay(NUM_SR_CONTACTS);

    bool is_open_button_pressed = false;

    bool is_close_button_unpressed = true;

    Timer *open_timer = new Timer(1.0, false);

    Timer *close_timer = new Timer(1.0, false);

    enum
    {
        NUM_DLR_CONTACTS = 1,
        DRL_LOCK = 0
    };

    /// Реле замыкания маршрута отправления
    Relay *departure_lock_relay = new Relay(NUM_DLR_CONTACTS);

    enum
    {
        NUM_SRS_NEUTRAL_CONTACTS = 2,
        NUM_SRS_PLUS_CONTACTS = 1,
        NUM_SRS_MINUS_CONTACTS = 1,

        SRS_N_RED = 0,
        SRS_N_YELLOW = 1,

        SRS_PLUS_YELLOW = 0,

        SRS_MINUS_GREEN = 0
    };

    /// Сигнальное реле светофора
    CombineRelay *semaphore_signal_relay = new CombineRelay(NUM_SRS_NEUTRAL_CONTACTS,
                                                            NUM_SRS_PLUS_CONTACTS,
                                                            NUM_SRS_MINUS_CONTACTS);

    enum
    {
        NUM_RCR_CONTACTS = 2,
        RCR_SR_CTRL = 0,
        RCR_SRS_CTRL = 1
    };

    /// Контрольное маршрутное реле
    Relay *route_control_relay = new Relay(NUM_RCR_CONTACTS);

    enum
    {
        NUM_YR_CONTACTS = 2,
        YR_SR_CTRL = 0,
        YR_SRS_PLUS = 1
    };

    /// Реле контроля первого участка удаления
    Relay *yellow_relay = new Relay(NUM_YR_CONTACTS);

    enum
    {
        NUM_GR_CONTACTS = 2,
        GR_SRS_MINUS = 0,
        GR_SRS_PLUS = 1
    };

    /// Реле контроля второго участка удаления
    Relay *green_relay = new Relay(NUM_GR_CONTACTS);

    enum
    {
        NUM_FWD_WR_CONTACTS = 1,
        FWD_BUSY = 0
    };

    /// Путевое реле стрелочного участка за светофором
    Relay *fwd_way_relay = new Relay(NUM_FWD_WR_CONTACTS);

    enum
    {
        NUM_AR_CONTACTS = 1,
        AR_OPEN = 0
    };

    /// Указательное реле, для связи с предыдущим входным светофором
    Relay *allow_relay = new Relay(NUM_AR_CONTACTS);

    double U_bat = 12.0;

    bool is_SR_ON = false;

    bool is_DRL_ON = false;

    bool is_SRS_ON = false;

    bool is_AR_ON = false;

    enum
    {
        NUM_SSR_CONTACTS = 2,
        SSR_GREEN = 0,
        SSR_YELLOW = 1
    };

    /// Боковое сигнальное реле (желтый мигающий для предвходного)
    Relay *side_signal_relay = new Relay(NUM_SSR_CONTACTS);

    /// Таймер мигания желтого
    Timer *blink_timer = new Timer(0.75, false);

    /// Контакт мигания
    bool blink_contact = true;

    /// Линейное реле, для связи со следующим светофором
    enum
    {
        NUM_LINE_NEUTRAL_CONTACTS = 1,
        NUM_LINE_PLUS_CONTACTS = 1,
        NUM_LINE_MINUS_CONTACTS = 0,

        LINE_N_YELLOW = 0,
        LINE_PLUS_GREEN = 0
    };

    CombineRelay *line_relay = new CombineRelay(NUM_LINE_NEUTRAL_CONTACTS,
                                                NUM_LINE_PLUS_CONTACTS,
                                                NUM_LINE_MINUS_CONTACTS);

    Signal *next_signal = Q_NULLPTR;

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override;

    void lens_control();

    void fwd_way_busy_control();

    void removal_area_control();

    void route_control(Signal **next_signal);

    void relay_control();

    void alsn_control();

private slots:

    void slotOpenTimer();

    void slotCloseTimer();

    void slotBlinkTimer();
};

#endif
