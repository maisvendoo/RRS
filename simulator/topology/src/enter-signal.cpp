#include    <enter-signal.h>
#include    <switch.h>
#include    <Journal.h>

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
    side_signal_relay->setInitContactState(SSR_TOP_YELLOW, false);
    side_signal_relay->setInitContactState(SSR_BOTTOM_YELLOW, false);

    direct_signal_relay->read_config("combine-reley");
    direct_signal_relay->setInitContactState(DSR_TOP_YELLOW, true);
    direct_signal_relay->setInitContactState(DSR_GREEN, false);

    bwd_way_relay->read_config("combine-relay");
    bwd_way_relay->setInitContactState(BWD_BUSY_RED, false);

    fwd_way_relay->read_config("combine-relay");
    fwd_way_relay->setInitContactState(FWD_BUSY_PLUS, false);
    fwd_way_relay->setInitContactState(FWD_BUSY_MINUS, true);
    fwd_way_relay->setInitContactState(FWD_BUSY_CLOSE, false);

    signal_relay->read_config("combine-relay");
    signal_relay->setInitContactState(SR_SELF_LOCK, false);
    signal_relay->setInitContactState(SR_MSR_SSR_CTRL, false);
    signal_relay->setInitContactState(SR_ALR_CTRL, false);
    signal_relay->setInitContactState(SR_PLUS, false);
    signal_relay->setInitContactState(SR_MINUS, true);

    arrival_lock_relay->read_config("combine-relay");
    arrival_lock_relay->setInitContactState(ALR_MSR_SSR_CTRL, false);

    route_control_relay->read_config("combine-relay");
    route_control_relay->setInitContactState(RCR_SR_CTRL, false);
    route_control_relay->setInitContactState(RCR_MSR_SSR_CTRL, false);
    route_control_relay->setInitContactState(RCR_DSR_CTRL, false);

    connect(open_timer, &Timer::process, this, &EnterSignal::slotOpenTimer);
    connect(close_timer, &Timer::process, this, &EnterSignal::slotCloseTimer);
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

    lens_state[YELLOW_LENS] = (side_signal_relay->getContactState(SSR_TOP_YELLOW) &&
                               main_signal_relay->getContactState(MSR_RED)) ||
                              (main_signal_relay->getContactState(MSR_YELLOW) && direct_signal_relay->getContactState(DSR_TOP_YELLOW));

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
void EnterSignal::busy_control()
{
    bwd_way_relay->setVoltage(U_bat * static_cast<double>(!is_bwd_busy));

    fwd_way_relay->setVoltage(U_bat * static_cast<double>(!is_fwd_busy));

    double U_line_prev_old = U_line_prev;

    double is_line_ON = static_cast<double>(bwd_way_relay->getContactState(BWD_BUSY_RED));
    double is_line_plus = static_cast<double>(signal_relay->getContactState(SR_PLUS));
    double is_line_minus = static_cast<double>(signal_relay->getContactState(SR_MINUS));


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
void EnterSignal::relay_control()
{
    // Собираем цепь контрольного маршрутного реле

    bool is_RCR_ON_old = is_RCR_ON;

    is_RCR_ON = is_route_free(conn);

    if (is_RCR_ON != is_RCR_ON_old)
    {
        if (is_RCR_ON)
            Journal::instance()->info("Route control relay is ON");
        else
            Journal::instance()->info("Route control relay is OFF");
    }

    route_control_relay->setVoltage(U_bat * static_cast<double>(is_RCR_ON));

    // Собираем цепь сигнального реле

    // Состояние провода кнопочного блока "Открыть/Закрыть"
    bool is_button_wire_ON = is_open_button_pressed ||
                             (is_close_button_nopressed && signal_relay->getContactState(SR_SELF_LOCK));

    bool is_SR_ON_old = is_SR_ON;

    is_SR_ON = is_button_wire_ON && fwd_way_relay->getContactState(FWD_BUSY_CLOSE) &&
               route_control_relay->getContactState(RCR_SR_CTRL);

    if (is_SR_ON != is_SR_ON_old)
    {
        if (is_SR_ON)
            Journal::instance()->info("Signal relay is ON");
        else
            Journal::instance()->info("Signal relay is OFF");
    }

    signal_relay->setVoltage(U_bat * static_cast<double>(is_SR_ON));

    // Цепь реле замыкания маршрута приема
    bool is_ALR_ON_old = is_ALR_ON;

    is_ALR_ON = signal_relay->getContactState(SR_ALR_CTRL);

    if (is_ALR_ON != is_ALR_ON_old)
    {
        if (is_ALR_ON)
            Journal::instance()->info("Arrival lock relay is ON");
        else
            Journal::instance()->info("Arrival lock relay is OFF");
    }

    arrival_lock_relay->setVoltage(U_bat * static_cast<double>(is_ALR_ON));

    // Цепи главного и бокового сигнальных реле

    // Состояние общего провода питания этих реле
    bool is_common_wire_ON = arrival_lock_relay->getContactState(ALR_MSR_SSR_CTRL) &&
                             signal_relay->getContactState(SR_MSR_SSR_CTRL) &&
                             route_control_relay->getContactState(RCR_MSR_SSR_CTRL);

    // Проверяем, стоят ли стрелки на бок
    bool is_minus = is_switch_minus(conn);

    bool is_MSR_ON_old = is_MSR_ON;

    is_MSR_ON = (!is_minus) && is_common_wire_ON;

    if (is_MSR_ON != is_MSR_ON_old)
    {
        if (is_MSR_ON)
            Journal::instance()->info("Main signal relay is ON");
        else
            Journal::instance()->info("Main signal relay is OFF");
    }

    bool is_SSR_ON_old = is_SSR_ON;

    is_SSR_ON = (is_minus) && is_common_wire_ON;

    if (is_SSR_ON != is_SSR_ON_old)
    {
        if (is_SSR_ON)
            Journal::instance()->info("Side signal relay is ON");
        else
            Journal::instance()->info("Side signal relay is OFF");
    }

    main_signal_relay->setVoltage(U_bat * static_cast<double>(is_MSR_ON));

    side_signal_relay->setVoltage(U_bat * static_cast<double>(is_SSR_ON));

    // Питание указательного реле выходного сигнала
    exit_signal_relay->setVoltage(U_line);

    // Цепь сигнального реле сквозного пропуска
    bool is_DSR_ON = exit_signal_relay->getContactState(ESR_DSR_CTRL) &&
                     route_control_relay->getContactState(RCR_DSR_CTRL);

    direct_signal_relay->setVoltage(U_bat * static_cast<double>(is_DSR_ON));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool EnterSignal::is_route_free(Connector *conn)
{
    if (conn == Q_NULLPTR)
    {
        return false;
    }

    bool is_free = true;

    Connector *cur_conn = conn;

    while (true)
    {
        // Смотрим траекторию за текущим коннектором
        Trajectory *traj = Q_NULLPTR;

        if (this->getDirection() == 1)
        {
            traj = cur_conn->getFwdTraj();
        }
        else
        {
            traj = cur_conn->getBwdTraj();
        }

        // Выходим, если за конектором нет валидной траектории - ехать некуда
        if (traj == Q_NULLPTR)
        {
            return false;
        }

        // Проверяем занятость следующей траектории
        is_free = is_free && (!traj->isBusy());

        // Ищем следующий коннектор
        if (this->getDirection() == 1)
        {
            cur_conn = traj->getFwdConnector();
        }
        else
        {
            cur_conn = traj->getBwdConnector();
        }

        // Если его нет, выходим стекущим результатом
        if (cur_conn == Q_NULLPTR)
        {
            break;
        }

        // Смотрим сигнал на следующем коннекторе
        Signal *signal = cur_conn->getSignal();

        // Продолжаем цикл, возможно попалась стрелка
        if (signal == Q_NULLPTR)
        {
            continue;
        }

        if (signal->getDirection() != this->getDirection())
        {
            continue;
        }

        break;
    }

    return is_free;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool EnterSignal::is_switch_minus(Connector *conn)
{
    if (conn == Q_NULLPTR)
    {
        return false;
    }

    bool is_minus = false;

    Switch *cur_sw = dynamic_cast<Switch *>(conn);

    while (true)
    {
        if (cur_sw == Q_NULLPTR)
        {
            return is_minus;
        }

        // проверка, стоят ли стрелки на бок
        is_minus = is_minus ||
                   (cur_sw->getStateFwd() == -1) ||
                   (cur_sw->getStateBwd() == -1);

        // Если одна на бок - выходим, дальнейшие проверки не имеют смысла
        if (is_minus)
        {
            return is_minus;
        }

        // проверяем, есть ли траектория дальше
        Trajectory *traj = Q_NULLPTR;

        if (this->getDirection() == 1)
        {
            traj = cur_sw->getFwdTraj();
        }
        else
        {
            traj = cur_sw->getBwdTraj();
        }

        if (traj == Q_NULLPTR)
        {
            break;
        }

        // получаем следующий коннектор
        if (this->getDirection() == 1)
        {
            cur_sw = dynamic_cast<Switch *>(traj->getFwdConnector());
        }
        else
        {
            cur_sw = dynamic_cast<Switch *>(traj->getBwdConnector());
        }

        if (cur_sw == Q_NULLPTR)
        {
            continue;
        }

        Signal *signal = cur_sw->getSignal();

        if (signal == Q_NULLPTR)
        {
            continue;
        }

        if (signal->getDirection() != this->getDirection())
        {
            continue;
        }

        break;
    }

    return is_minus;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::preStep(state_vector_t &Y, double t)
{
    lens_control();

    busy_control();

    relay_control();
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

    Journal::instance()->info("Released open button");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::slotCloseTimer()
{
    is_close_button_nopressed = true;
    close_timer->stop();

    Journal::instance()->info("Released close button");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::slotPressOpen()
{
    is_open_button_pressed = true;
    open_timer->start();

    Journal::instance()->info("Pressed open button");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EnterSignal::slotPressClose()
{
    is_close_button_nopressed = false;
    close_timer->start();

    Journal::instance()->info("Pressed close button");
}
