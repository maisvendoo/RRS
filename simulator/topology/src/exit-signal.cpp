#include    "exit-signal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ExitSignal::ExitSignal(QObject *parent) : StationSignal(parent)
{
    connect(blink_timer, &Timer::process, this, &ExitSignal::slotBlinkTimer);

    semaphore_signal_relay->read_config("combine-relay");
    semaphore_signal_relay->setInitContactState(SRS_N_RED, true);
    semaphore_signal_relay->setInitContactState(SRS_N_ALLOW, false);
    semaphore_signal_relay->setInitPlusContactState(SRS_PLUS_GREEN, false);
    semaphore_signal_relay->setInitMinusContactState(SRS_MINUS_YELLOW, false);

    side_signal_relay->read_config("combine-relay");
    side_signal_relay->setInitContactState(SSR_GREEN, true);
    side_signal_relay->setInitContactState(SSR_YELLOW, false);
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
    StationSignal::step(t, dt);

    blink_timer->step(t, dt);

    semaphore_signal_relay->step(t, dt);
    side_signal_relay->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::preStep(double t)
{
    (void)t;

    relay_control();

    yellow_blink_control();

    lens_control();

    alsn_control();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::relay_control()
{

    // Сигнальное реле светофора
    bool is_SRS_ON = control_relay->getContactState(CR_ALLOW_ROUTE) &&
                     signal_relay->getContactState(SR_OPENED) &&
                     lock_relay->getContactState(LR_ROUTE_LOCKED);

    semaphore_signal_relay->setVoltage(static_cast<double>(is_SRS_ON) * U_line);


    double is_line_plus = static_cast<double>(signal_relay->getContactState(SR_OPENED));
    double is_line_minus = static_cast<double>(signal_relay->getContactState(SR_CLOSED));

    // Формируем напряжение, подаваемое на линейное реле предыдущего светофора
    U_line_prev = U_bat * (is_line_plus - is_line_minus);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::yellow_blink_control()
{
    // Включение реле мигания от следующего светофора
    side_signal_relay->setVoltage(U_side);

    // Управление таймером мигания
    if (side_signal_relay->getContactState(SSR_YELLOW))
    {
        blink_timer->start();
    }
    else
    {
        blink_timer->stop();
        blink_contact = true;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::lens_control()
{
    lens_state[RED_LENS] = semaphore_signal_relay->getContactState(SRS_N_RED);

    lens_state[YELLOW_LENS] = semaphore_signal_relay->getContactState(SRS_N_ALLOW) &&
                              (semaphore_signal_relay->getMinusContactState(SRS_MINUS_YELLOW) ||
                              (blink_contact && side_signal_relay->getContactState(SSR_YELLOW)));

    lens_state[GREEN_LENS] = semaphore_signal_relay->getContactState(SRS_N_ALLOW) &&
                             semaphore_signal_relay->getPlusContactState(SRS_PLUS_GREEN) &&
                             side_signal_relay->getContactState(SSR_GREEN);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::alsn_control()
{
    bool is_ALSN_RY_ON = semaphore_signal_relay->getContactState(SRS_N_RED);

    alsn_RY_relay->setVoltage(U_bat * static_cast<double>(is_ALSN_RY_ON));

    bool is_ALSN_Y_ON = semaphore_signal_relay->getContactState(SRS_N_ALLOW) &&
                        semaphore_signal_relay->getMinusContactState(SRS_MINUS_YELLOW);

    alsn_Y_relay->setVoltage(U_bat * static_cast<double>(is_ALSN_Y_ON));

    bool is_ALSN_G_ON = semaphore_signal_relay->getContactState(SRS_N_ALLOW) &&
                        semaphore_signal_relay->getPlusContactState(SRS_PLUS_GREEN);

    alsn_G_relay->setVoltage(U_bat * static_cast<double>(is_ALSN_G_ON));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::slotBlinkTimer()
{
    blink_contact = !blink_contact;
}
