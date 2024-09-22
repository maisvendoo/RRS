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

    departure_lock_relay->read_config("combine-relay");
    departure_lock_relay->setInitContactState(DRL_LOCK, true);

    semaphore_signal_relay->read_config("combine-relay");
    semaphore_signal_relay->setInitContactState(SRS_N_RED, true);
    semaphore_signal_relay->setInitContactState(SRS_N_YELLOW, false);
    semaphore_signal_relay->setInitPlusContactState(SRS_PLUS_YELLOW, false);
    semaphore_signal_relay->setInitMinusContactState(SRS_MINUS_GREEN, false);

    route_control_relay->read_config("combine-relay");
    route_control_relay->setInitContactState(RCR_SR_CTRL, false);
    route_control_relay->setInitContactState(RCR_SRS_CTRL, false);

    yellow_relay->read_config("combine-relay");
    yellow_relay->setInitContactState(YR_SR_CTRL, false);
    yellow_relay->setInitContactState(YR_SRS_PLUS, false);

    green_relay->read_config("combine-relay");
    green_relay->setInitContactState(GR_SRS_MINUS, false);
    green_relay->setInitContactState(GER_SRS_PLUS, true);

    fwd_way_relay->read_config("combine-relay");
    fwd_way_relay->setInitContactState(FWD_BUSY, false);

    allow_relay->read_config("combine-relay");
    allow_relay->setInitContactState(AR_OPEN, false);
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

    departure_lock_relay->step(t, dt);

    semaphore_signal_relay->step(t, dt);

    route_control_relay->step(t, dt);

    yellow_relay->step(t, dt);

    green_relay->step(t, dt);

    fwd_way_relay->step(t, dt);

    allow_relay->step(t, dt);
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
    lens_control();
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
void ExitSignal::lens_control()
{
    lens_state[RED_LENS] = semaphore_signal_relay->getContactState(SRS_N_RED);

    lens_state[YELLOW_LENS] = semaphore_signal_relay->getContactState(SRS_N_YELLOW) &&
                              semaphore_signal_relay->getPlusContactState(SRS_PLUS_YELLOW);

    lens_state[GREEN_LENS] = semaphore_signal_relay->getContactState(SRS_N_YELLOW) &&
                             semaphore_signal_relay->getMinusContactState(SRS_MINUS_GREEN);
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
