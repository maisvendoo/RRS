#include    <exit-signal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ExitSignal::ExitSignal(QObject *parent) : Signal(parent)
{
    signal_relay->read_config("combine-relay");
    signal_relay->setInitContactState(SR_SELF, false);
    signal_relay->setInitContactState(SR_DLR_CTRL, false);
    signal_relay->setInitContactState(SR_SRS_CTRL, false);

    connect(open_timer, &Timer::process, this, &ExitSignal::slotOpenTimer);
    connect(close_timer, &Timer::process, this, &ExitSignal::slotCloseTimer);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ExitSignal::~ExitSignal()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::step(double t, double dt)
{
    Signal::step(t, dt);

    signal_relay->step(t, dt);

    open_timer->step(t, dt);
    close_timer->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::slotPressOpen()
{
    is_open_button_pressed = true;
    open_timer->start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::slotPressClose()
{
    is_close_button_unpressed = false;
    close_timer->start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::preStep(state_vector_t &Y, double t)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::slotOpenTimer()
{
    is_open_button_pressed = false;
    open_timer->stop();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::slotCloseTimer()
{
    is_close_button_unpressed = true;
    close_timer->stop();
}
