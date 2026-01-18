#include    "enter-signal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EnterSignal::EnterSignal(QObject *parent) : StationSignal(parent)
{
    main_signal_relay->read_config("combine-relay");
    main_signal_relay->setInitContactState(MSR_RED, true);
    main_signal_relay->setInitContactState(MSR_YELLOW, false);
    main_signal_relay->setInitContactState(MSR_GREEN, false);
    main_signal_relay->setInitContactState(MSR_BLINK, false);

    side_signal_relay->read_config("combine-relay");
    side_signal_relay->setInitContactState(SSR_RED, true);
    side_signal_relay->setInitContactState(SSR_TOP_YELLOW, false);
    side_signal_relay->setInitContactState(SSR_BOTTOM_YELLOW, false);
    side_signal_relay->setInitContactState(SSR_SIDE, false);
    side_signal_relay->setInitContactState(SSR_BLINK, false);

    direct_signal_relay->read_config("combine-relay");
    direct_signal_relay->setInitContactState(DSR_TOP_YELLOW, true);
    direct_signal_relay->setInitContactState(DSR_GREEN, false);
    direct_signal_relay->setInitContactState(DSR_BLINK, false);

    blink_relay->read_config("combine-relay");
    blink_relay->setInitContactState(BLINK_GREEN, true);
    blink_relay->setInitContactState(BLINK_YELLOW, false);
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
    StationSignal::step(t, dt);

    main_signal_relay->step(t, dt);
    side_signal_relay->step(t, dt);
    direct_signal_relay->step(t, dt);

    blink_relay->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::preStep(double t)
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
void EnterSignal::relay_control()
{
    // Цепи главного и бокового сигнальных реле
    bool is_common_wire_ON = control_relay->getContactState(CR_ALLOW_ROUTE) &&
                             signal_relay->getContactState(SR_OPENED) &&
                             lock_relay->getContactState(LR_NEUTRAL_ROUTE_LOCKED);

    bool is_MSR_ON = is_common_wire_ON && (switches_state == SWITCHES_STRAIGHT);
    bool is_SSR_ON = is_common_wire_ON && (switches_state == SWITCHES_SIDE);

    main_signal_relay->setVoltage(static_cast<double>(is_MSR_ON) * U_bat);
    side_signal_relay->setVoltage(static_cast<double>(is_SSR_ON) * U_bat);


    // Цепь сигнального реле сквозного пропуска
    bool is_DSR_ON = control_relay->getContactState(CR_ALLOW_ROUTE);

    // Питание от следующего сигнала
    double U_line_plus = max(0.0, U_line);

    direct_signal_relay->setVoltage(U_line_plus * static_cast<double>(is_DSR_ON));


    double is_line_plus = static_cast<double>(signal_relay->getContactState(SR_OPENED));
    double is_line_minus = static_cast<double>(signal_relay->getContactState(SR_CLOSED));

    // Формируем напряжение, подаваемое на линейное реле предыдущего светофора
    U_line_prev = (is_line_plus - is_line_minus) * U_bat;

    // Формируем напряжение, подаваемое на боковое сигнальное реле предыдущего светофора
    U_side_prev = static_cast<double>(side_signal_relay->getContactState(SSR_SIDE)) * U_bat;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::yellow_blink_control()
{
    // Цепь реле мигания от реле сквозного пропуска
    bool is_blink_ON = direct_signal_relay->getContactState(DSR_BLINK);
    if (main_signal_relay->getContactState(MSR_BLINK))
    {
        // Если маршрут прямо, питание реле мигания от бокового реле следующего сигнала
        blink_relay->setVoltage(static_cast<double>(is_blink_ON) * U_side);
    }
    else
    {
        // Проверяем маршрут с отклонением по стрелкам
        is_blink_ON &= side_signal_relay->getContactState(SSR_BLINK);
        blink_relay->setVoltage(static_cast<double>(is_blink_ON) * U_bat);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::lens_control()
{
    lens_state[GREEN_LENS] = main_signal_relay->getContactState(MSR_GREEN) &&
                             direct_signal_relay->getContactState(DSR_GREEN) &&
                             blink_relay->getContactState(BLINK_GREEN);

    is_yellow_wire_ON = (main_signal_relay->getContactState(MSR_YELLOW) ||
                         side_signal_relay->getContactState(SSR_TOP_YELLOW)) &&
                         direct_signal_relay->getContactState(DSR_TOP_YELLOW);

    lens_state[YELLOW_LENS] = (is_yellow_wire_ON ||
                               (blink_contact && blink_relay->getContactState(BLINK_YELLOW)));

    lens_state[RED_LENS] = signal_relay_shunt->getContactState(SRS_CLOSED) &&
                           side_signal_relay->getContactState(SSR_RED) &&
                           main_signal_relay->getContactState(MSR_RED);

    lens_state[BOTTOM_YELLOW_LENS] = side_signal_relay->getContactState(SSR_BOTTOM_YELLOW);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::alsn_control()
{
    bool is_ALSN_RY_ON = lens_state[RED_LENS];

    alsn_RY_relay->setVoltage(static_cast<double>(is_ALSN_RY_ON) * U_bat);


    bool is_ALSN_G_ON = lens_state[GREEN_LENS] ||
                        (main_signal_relay->getContactState(MSR_BLINK) &&
                         blink_relay->getContactState(BLINK_YELLOW));

    alsn_G_relay->setVoltage(static_cast<double>(is_ALSN_G_ON) * U_bat);


    bool is_ALSN_Y_ON = lens_state[BOTTOM_YELLOW_LENS] || is_yellow_wire_ON;

    alsn_Y_relay->setVoltage(static_cast<double>(is_ALSN_Y_ON) * U_bat);
}
