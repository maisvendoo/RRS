#include    <enter-signal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EnterSignal::EnterSignal(QObject *parent) : Signal(parent)
{
    main_signal_relay->read_config("combine-relay");
    main_signal_relay->setInitContactState(MSR_RED, true);
    main_signal_relay->setInitContactState(MSR_YELLOW, false);

    side_signal_relay->read_config("combine-relay");
    side_signal_relay->setInitContactState(SSR_RED, true);
    side_signal_relay->setInitContactState(SSR_TOP_YELLOW, true);
    side_signal_relay->setInitContactState(SSR_BOTTOM_YELLOW, false);

    direct_signal_relay->read_config("combine-reley");
    direct_signal_relay->setInitContactState(DSR_TOP_YELLOW, true);
    direct_signal_relay->setInitContactState(DSR_GREEN, false);

    fwd_way_relay->read_config("combine-relay");
    fwd_way_relay->setInitContactState(FWD_BUSY_RED, false);

    bwd_way_relay->read_config("combine-relay");
    bwd_way_relay->setInitContactState(BWD_BUSY_PLUS, false);
    bwd_way_relay->setInitContactState(BWD_BUSY_MINUS, true);
    bwd_way_relay->setInitContactState(BWD_BUSY_CLOSE, false);
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

    fwd_way_relay->step(t, dt);
    bwd_way_relay->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::lens_control()
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
void EnterSignal::busy_control()
{
    bwd_way_relay->setVoltage(U_bat * static_cast<double>(!is_bwd_busy));

    fwd_way_relay->setVoltage(U_bat * static_cast<double>(!is_fwd_busy));

    double U_line_prev_old = U_line_prev;

    double is_line_ON = static_cast<double>(fwd_way_relay->getContactState(FWD_BUSY_RED));
    double is_line_plus = static_cast<double>(bwd_way_relay->getContactState(BWD_BUSY_PLUS));
    double is_line_minus = static_cast<double>(bwd_way_relay->getContactState(BWD_BUSY_MINUS));

    U_line_prev = U_bat * (is_line_plus - is_line_minus) * is_line_ON;

    // Если напряжение питание изменилось
    if (qAbs(U_line_prev - U_line_prev_old) >= 1.0)
    {
        // Устанавливаем новое значение на линии предыдущего светофора
        emit sendLineVoltage(U_line_prev);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::preStep(state_vector_t &Y, double t)
{
    lens_control();

    busy_control();
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
