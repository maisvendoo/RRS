#include    <enter-signal.h>
#include    <switch.h>
#include    <Journal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EnterSignal::EnterSignal(QObject *parent) : StationSignal(parent)
{
    connect(blink_timer, &Timer::process, this, &EnterSignal::slotOnBlinkTimer);

    main_signal_relay->read_config("combine-relay");
    main_signal_relay->setInitContactState(MSR_RED, true);
    main_signal_relay->setInitContactState(MSR_YELLOW, false);
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

    blink_timer->step(t, dt);

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

    bool is_switches_side = false;
    check_route(is_switches_side);

    relay_control(is_switches_side);

    yellow_blink_control();

    lens_control();

    alsn_control();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::check_route(bool& is_switches_side)
{
    // Начинаем с коннектора, к которому относится светофор
    Connector *cur_conn = conn;

    if (!cur_conn)
    {
        U_way = 0.0;
        U_line = 0.0;
        U_side = 0.0;
        return;
    }

    while (true)
    {
        // Смотрим траекторию за текущим коннектором
        Trajectory* traj = (signal_dir == 1) ? cur_conn->getFwdTraj() : cur_conn->getBwdTraj();

        if (!traj)
        {
            U_way = 0.0;
            U_line = 0.0;
            U_side = 0.0;
            return;
        }

        // Занятость траектории
        if (traj->isBusy())
        {
            U_way = 0.0;
            U_line = 0.0;
            U_side = 0.0;
            return;
        }

        // Смотрим следующий коннектор
        cur_conn = (signal_dir == 1) ? traj->getFwdConnector() : traj->getBwdConnector();

        if (!cur_conn)
        {
            U_way = 0.0;
            U_line = 0.0;
            U_side = 0.0;
            return;
        }

        // Контроль взреза стрелки: смотрим траекторию перед следующим коннектором
        Trajectory* prev = (signal_dir == 1) ? cur_conn->getBwdTraj() : cur_conn->getFwdTraj();

        if (traj != prev)
        {
            U_way = 0.0;
            U_line = 0.0;
            U_side = 0.0;
            return;
        }

        // Смотрим отклонение по стрелке
        if (Switch* sw = dynamic_cast<Switch*>(cur_conn))
        {
            is_switches_side |= (sw->getStateBwd() < 0);
            is_switches_side |= (sw->getStateFwd() < 0);
        }

        // Смотрим сигнал на следующем коннекторе
        Signal* signal = (signal_dir == 1) ? cur_conn->getSignalFwd() : cur_conn->getSignalBwd();

        if (signal)
        {
            if (TrainSignal* ts = dynamic_cast<TrainSignal*>(signal))
            {
                // Замыкание цепи на путевое реле в релейном шкафу следующего светофора
                U_way = U_bat;
                // Линейное реле питается от линии следующего светофора
                U_line = ts->getLineVoltage();
                // Боковое сигнальное реле питается от линии следующего светофора
                U_side = ts->getSideVoltage();

                // Если сигнал закрыт, отключаем АЛСН-код от следующего светофора
                ts->allowTransmitALSN(!lens_state[RED_LENS]);
                return;
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::relay_control(bool is_switches_side)
{

    // Цепи главного и бокового сигнальных реле
    bool is_common_wire_ON = control_relay->getContactState(CR_ALLOW_ROUTE) &&
                             signal_relay->getContactState(SR_OPENED) &&
                             lock_relay->getContactState(LR_ROUTE_LOCKED);

    bool is_MSR_ON = is_common_wire_ON && (!is_switches_side);
    bool is_SSR_ON = is_common_wire_ON && is_switches_side;

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
    // Цепь реле мигания
    if (main_signal_relay->getContactState(MSR_BLINK))
    {
        // Если маршрут прямо, питание реле мигания от бокового реле следующего сигнала
        blink_relay->setVoltage(U_side);
    }
    else
    {
        // Если маршрут с отклонением по стрелкам, питание реле мигания от реле сквозного пропуска
        bool is_blink_ON = side_signal_relay->getContactState(SSR_BLINK) &&
                           direct_signal_relay->getContactState(DSR_BLINK);
        blink_relay->setVoltage(static_cast<double>(is_blink_ON) * U_bat);
    }

    // Управление таймером мигания
    if (blink_relay->getContactState(BLINK_YELLOW))
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
void EnterSignal::lens_control()
{
    lens_state[GREEN_LENS] = direct_signal_relay->getContactState(DSR_GREEN) &&
                             main_signal_relay->getContactState(MSR_YELLOW);

    is_yellow_wire_ON = (side_signal_relay->getContactState(SSR_TOP_YELLOW) &&
                            main_signal_relay->getContactState(MSR_RED)) ||
                           (main_signal_relay->getContactState(MSR_YELLOW) &&
                             direct_signal_relay->getContactState(DSR_TOP_YELLOW));

    lens_state[YELLOW_LENS] = (is_yellow_wire_ON && blink_contact) ||
                              (blink_contact && direct_signal_relay->getContactState(DSR_BLINK) &&
                               blink_relay->getContactState(BLINK_YELLOW) &&
                              (main_signal_relay->getContactState(MSR_BLINK) ||
                               side_signal_relay->getContactState(SSR_BLINK)));

    lens_state[RED_LENS] = side_signal_relay->getContactState(SSR_RED) &&
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
                        (blink_relay->getContactState(BLINK_YELLOW) &&
                         !side_signal_relay->getContactState(SSR_BOTTOM_YELLOW) * U_bat);

    alsn_G_relay->setVoltage(static_cast<double>(is_ALSN_G_ON) * U_bat);


    bool is_ALSN_Y_ON = is_yellow_wire_ON;

    alsn_Y_relay->setVoltage(static_cast<double>(is_ALSN_Y_ON) * U_bat);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::slotOnBlinkTimer()
{
    blink_contact = !blink_contact;
}
