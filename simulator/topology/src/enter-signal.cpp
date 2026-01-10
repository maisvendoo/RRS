#include    <enter-signal.h>
#include    <switch.h>
#include    <Journal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EnterSignal::EnterSignal(QObject *parent) : Signal(parent)
{
    connect(open_timer, &Timer::process, this, &EnterSignal::slotOpenTimer);
    connect(close_timer, &Timer::process, this, &EnterSignal::slotCloseTimer);

    connect(blink_timer, &Timer::process, this, &EnterSignal::slotOnBlinkTimer);

    route_control_relay->read_config("combine-relay");
    route_control_relay->setInitContactState(RCR_SR_CTRL, false);
    route_control_relay->setInitContactState(RCR_MSR_SSR_CTRL, false);
    route_control_relay->setInitContactState(RCR_DSR_CTRL, false);

    signal_relay->read_config("combine-relay");
    signal_relay->setInitContactState(SR_SELF_LOCK, false);
    signal_relay->setInitContactState(SR_MSR_SSR_CTRL, false);
    signal_relay->setInitContactState(SR_ALR_CTRL, false);
    signal_relay->setInitContactState(SR_PLUS, false);
    signal_relay->setInitContactState(SR_MINUS, true);

    arrival_lock_relay->read_config("combine-relay");
    arrival_lock_relay->setInitContactState(ALR_MSR_SSR_CTRL, false);

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
    Signal::step(t, dt);

    open_timer->step(t, dt);
    close_timer->step(t, dt);
    blink_timer->step(t, dt);

    route_control_relay->step(t, dt);
    signal_relay->step(t, dt);
    arrival_lock_relay->step(t, dt);
    main_signal_relay->step(t, dt);
    side_signal_relay->step(t, dt);
    direct_signal_relay->step(t, dt);

    blink_relay->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::slotPressOpen()
{
    is_open_button_pressed = true;
    open_timer->start();

    Journal::instance()->info("Pressed open button for signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::slotPressClose()
{
    is_close_button_nopressed = false;
    close_timer->start();

    Journal::instance()->info("Pressed close button for signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::preStep(state_vector_t &Y, double t)
{
    (void)Y;
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
void EnterSignal::ode_system(const state_vector_t &Y,
                             state_vector_t &dYdt,
                             double t)
{
    (void)Y;
    (void)dYdt;
    (void)t;
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
            // Замыкание цепи на путевое реле в релейном шкафу следующего светофора
            U_way = U_bat;
            // Линейное реле питается от линии следующего светофора
            U_line = signal->getLineVoltage();
            // Боковое сигнальное реле питается от линии следующего светофора
            U_side = signal->getSideVoltage();

            // Если сигнал закрыт, отключаем АЛСН-код от следующего светофора
            signal->allowTransmitALSN(!lens_state[RED_LENS]);
            return;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::relay_control(bool is_switches_side)
{
    // Цепь контрольного маршрутного реле
    route_control_relay->setVoltage(U_way);


    // Цепь сигнального реле
    // Состояние провода кнопочного блока "Открыть/Закрыть"
    bool is_SR_ON = is_open_button_pressed ||
                    (is_close_button_nopressed && signal_relay->getContactState(SR_SELF_LOCK));

    // Контакт контрольного маршрутного реле
    is_SR_ON &= route_control_relay->getContactState(RCR_SR_CTRL);

    signal_relay->setVoltage(U_bat * static_cast<double>(is_SR_ON));


    // Замыкание маршрута приема
    bool is_ALR_ON = signal_relay->getContactState(SR_ALR_CTRL);

    arrival_lock_relay->setVoltage(U_bat * static_cast<double>(is_ALR_ON));


    // Цепи главного и бокового сигнальных реле
    bool is_common_wire_ON = arrival_lock_relay->getContactState(ALR_MSR_SSR_CTRL) &&
                             signal_relay->getContactState(SR_MSR_SSR_CTRL) &&
                             route_control_relay->getContactState(RCR_MSR_SSR_CTRL);

    bool is_MSR_ON = is_common_wire_ON && (!is_switches_side);
    bool is_SSR_ON = is_common_wire_ON && is_switches_side;

    main_signal_relay->setVoltage(U_bat * static_cast<double>(is_MSR_ON));
    side_signal_relay->setVoltage(U_bat * static_cast<double>(is_SSR_ON));


    // Цепь сигнального реле сквозного пропуска
    bool is_DSR_ON = route_control_relay->getContactState(RCR_DSR_CTRL);

    // Питание от следующего сигнала
    double U_line_plus = max(0.0, U_line);

    direct_signal_relay->setVoltage(U_line_plus * static_cast<double>(is_DSR_ON));


    double is_line_plus = static_cast<double>(signal_relay->getContactState(SR_PLUS));
    double is_line_minus = static_cast<double>(signal_relay->getContactState(SR_MINUS));

    // Формируем напряжение, подаваемое на линейное реле предыдущего светофора
    U_line_prev = U_bat * (is_line_plus - is_line_minus);

    // Формируем напряжение, подаваемое на боковое сигнальное реле предыдущего светофора
    U_side_prev = U_bat * static_cast<double>(side_signal_relay->getContactState(SSR_SIDE));
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
        blink_relay->setVoltage(U_bat * static_cast<double>(is_blink_ON));
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
    old_lens_state = lens_state;

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

    if (lens_state != old_lens_state)
    {
        emit sendDataUpdate(this->serialize());
        Journal::instance()->info("Signal " + letter + ": Updated lens status");
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::alsn_control()
{
    if (!is_asln_transmit)
    {
        alsn_reset();
        return;
    }

    bool is_ALSN_RY_ON = lens_state[RED_LENS];

    alsn_RY_relay->setVoltage(U_bat * static_cast<double>(is_ALSN_RY_ON));

    alsn_state[ALSN_RY_LINE] = alsn_RY_relay->getContactState(ALSN_RY);

    bool is_ALSN_G_ON = lens_state[GREEN_LENS] ||
                        (blink_relay->getContactState(BLINK_YELLOW) &&
                         !side_signal_relay->getContactState(SSR_BOTTOM_YELLOW));

    alsn_G_relay->setVoltage(U_bat * static_cast<double>(is_ALSN_G_ON));

    alsn_state[ALSN_G_LINE] = alsn_G_relay->getContactState(ALSN_G);

    bool is_ALSN_Y_ON = is_yellow_wire_ON;

    alsn_Y_relay->setVoltage(U_bat * static_cast<double>(is_ALSN_Y_ON));

    alsn_state[ALSN_Y_LINE] = alsn_Y_relay->getContactState(ALSN_Y);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::slotOpenTimer()
{
    is_open_button_pressed = false;
    open_timer->stop();

    Journal::instance()->info("Released open button for signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::slotCloseTimer()
{
    is_close_button_nopressed = true;
    close_timer->stop();

    Journal::instance()->info("Released close button for signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::slotOnBlinkTimer()
{
    blink_contact = !blink_contact;
}
