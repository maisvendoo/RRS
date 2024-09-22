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

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override;

private slots:

    void slotOpenTimer();

    void slotCloseTimer();
};

#endif
