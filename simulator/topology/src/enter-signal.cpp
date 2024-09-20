#include    <enter-signal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EnterSignal::EnterSignal(QObject *parent) : Signal(parent)
{
    main_signal_relay->setInitContactState(MSR_RED, true);
    main_signal_relay->setInitContactState(MSR_YELLOW, false);

    side_signal_relay->setInitContactState(SSR_RED, true);
    side_signal_relay->setInitContactState(SSR_TOP_YELLOW, true);
    side_signal_relay->setInitContactState(SSR_BOTTOM_YELLOW, false);

    direct_signal_relay->setInitContactState(DSR_TOP_YELLOW, true);
    direct_signal_relay->setInitContactState(DSR_GREEN, false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EnterSignal::~EnterSignal()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::step(double t, double dt)
{
    Signal::step(t, dt);

    open_timer->step(t, dt);

    close_timer->step(t, dt);

    main_signal_relay->step(t, dt);
    side_signal_relay->step(t, dt);
    direct_signal_relay->step(t, dt);
    route_control_relay->step(t, dt);
    signal_relay->step(t, dt);
    arrival_lock_relay->step(t, dt);
    exit_signal_relay->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::preStep(state_vector_t &Y, double t)
{
    old_lens_state = lens_state;

    lens_state[GREEN_LENS] = direct_signal_relay->getContactState(DSR_GREEN) &&
                             main_signal_relay->getContactState(MSR_YELLOW);

    lens_state[YELLOW_LENS] = side_signal_relay->getContactState(SSR_TOP_YELLOW) &&
                              main_signal_relay->getContactState(MSR_RED);

    lens_state[RED_LENS] = side_signal_relay->getContactState(SSR_RED) &&
                           main_signal_relay->getContactState(MSR_RED);

    lens_state[BOTTOM_YELLOW_LENS] = side_signal_relay->getContactState(SSR_BOTTOM_YELLOW);

    if (lens_state != old_lens_state)
    {
        emit sendDataUpdate(this->serialize());
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::ode_system(const state_vector_t &Y,
                             state_vector_t &dYdt,
                             double t)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::slotOpenTimer()
{
    is_open_button_pressed = false;
    open_timer->stop();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::slotCloseTimer()
{
    is_close_button_nopressed = true;
    close_timer->stop();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::slotPressOpen()
{
    is_open_button_pressed = true;
    open_timer->start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::slotPressClose()
{
    is_close_button_nopressed = false;
    close_timer->start();
}
