#include    <exit-signal.h>
#include    <Journal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ExitSignal::ExitSignal(QObject *parent) : Signal(parent)
{
    connect(open_timer, &Timer::process, this, &ExitSignal::slotOpenTimer);
    connect(close_timer, &Timer::process, this, &ExitSignal::slotCloseTimer);

    connect(blink_timer, &Timer::process, this, &ExitSignal::slotBlinkTimer);

    route_control_relay->read_config("combine-relay");
    route_control_relay->setInitContactState(RCR_SR_CTRL, false);
    route_control_relay->setInitContactState(RCR_SRS_CTRL, false);

    signal_relay->read_config("combine-relay");
    signal_relay->setInitContactState(SR_SELF, false);
    signal_relay->setInitContactState(SR_DLR_CTRL, true);
    signal_relay->setInitContactState(SR_SRS_CTRL, false);
    signal_relay->setInitContactState(SR_PLUS, false);
    signal_relay->setInitContactState(SR_MINUS, true);

    departure_lock_relay->read_config("combine-relay");
    departure_lock_relay->setInitContactState(DRL_CTRL, true);

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
    Signal::step(t, dt);

    open_timer->step(t, dt);
    close_timer->step(t, dt);

    blink_timer->step(t, dt);

    route_control_relay->step(t, dt);

    signal_relay->step(t, dt);

    departure_lock_relay->step(t, dt);

    semaphore_signal_relay->step(t, dt);

    side_signal_relay->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::slotPressOpen()
{
    is_open_button_pressed = true;
    open_timer->start();

    Journal::instance()->info("Pressed open button for exit signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::slotPressClose()
{
    is_close_button_unpressed = false;
    close_timer->start();

    Journal::instance()->info("Pressed close button for exit signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::preStep(state_vector_t &Y, double t)
{
    (void)Y;
    (void)t;

    check_route();

    relay_control();

    yellow_blink_control();

    lens_control();

    alsn_control();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::ode_system(const state_vector_t &Y,
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
void ExitSignal::check_route()
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
void ExitSignal::relay_control()
{
    // Цепь контрольного маршрутного реле
    route_control_relay->setVoltage(U_way);

    // Цепь сигнального реле
    // Состояние провода кнопочного блока "Открыть/Закрыть"
    bool is_SR_ON = is_open_button_pressed ||
                    (is_close_button_unpressed && signal_relay->getContactState(SR_SELF));

    // Контакт контрольного маршрутного реле
    is_SR_ON &= route_control_relay->getContactState(RCR_SR_CTRL);

    signal_relay->setVoltage(static_cast<double>(is_SR_ON) * U_bat);


    // Замыкание маршрута отправления
    bool is_DRL_ON = signal_relay->getContactState(SR_DLR_CTRL);

    departure_lock_relay->setVoltage(static_cast<double>(is_DRL_ON) * U_bat);

    // Сигнальное реле светофора
    bool is_SRS_ON = departure_lock_relay->getContactState(DRL_CTRL) &&
                     signal_relay->getContactState(SR_SRS_CTRL) &&
                     route_control_relay->getContactState(RCR_SRS_CTRL);

    semaphore_signal_relay->setVoltage(static_cast<double>(is_SRS_ON) * U_line);


    double is_line_plus = static_cast<double>(signal_relay->getContactState(SR_PLUS));
    double is_line_minus = static_cast<double>(signal_relay->getContactState(SR_MINUS));

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
    old_lens_state = lens_state;

    lens_state[RED_LENS] = semaphore_signal_relay->getContactState(SRS_N_RED);

    lens_state[YELLOW_LENS] = semaphore_signal_relay->getContactState(SRS_N_ALLOW) &&
                              (semaphore_signal_relay->getPlusContactState(SRS_PLUS_GREEN) ||
                              (blink_contact && side_signal_relay->getContactState(SSR_YELLOW)));

    lens_state[GREEN_LENS] = semaphore_signal_relay->getContactState(SRS_N_ALLOW) &&
                             semaphore_signal_relay->getMinusContactState(SRS_MINUS_YELLOW) &&
                             side_signal_relay->getContactState(SSR_GREEN);

    if (lens_state != old_lens_state)
    {
        emit sendDataUpdate(this->serialize());
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::alsn_control()
{
    if (!is_asln_transmit)
    {
        alsn_reset();
        return;
    }

    bool is_ALSN_RY_ON = semaphore_signal_relay->getContactState(SRS_N_RED);

    alsn_RY_relay->setVoltage(U_bat * static_cast<double>(is_ALSN_RY_ON));

    alsn_state[ALSN_RY_LINE] = alsn_RY_relay->getContactState(ALSN_RY);

    bool is_ALSN_Y_ON = semaphore_signal_relay->getContactState(SRS_N_ALLOW) &&
                        semaphore_signal_relay->getPlusContactState(SRS_MINUS_YELLOW);

    alsn_Y_relay->setVoltage(U_bat * static_cast<double>(is_ALSN_Y_ON));

    alsn_state[ALSN_Y_LINE] = alsn_Y_relay->getContactState(ALSN_Y);

    bool is_ALSN_G_ON = lens_state[GREEN_LENS] ||
                        side_signal_relay->getContactState(SSR_YELLOW);

    alsn_G_relay->setVoltage(U_bat * static_cast<double>(is_ALSN_G_ON));

    alsn_state[ALSN_G_LINE] = alsn_G_relay->getContactState(ALSN_G);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::slotOpenTimer()
{
    is_open_button_pressed = false;
    open_timer->stop();

    Journal::instance()->info("Released open button for exit signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::slotCloseTimer()
{
    is_close_button_unpressed = true;
    close_timer->stop();

    Journal::instance()->info("Released close button for exit signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::slotBlinkTimer()
{
    blink_contact = !blink_contact;
}
