#include    <exit-signal.h>
#include    <Journal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ExitSignal::ExitSignal(QObject *parent) : Signal(parent)
{
    signal_relay->read_config("combine-relay");
    signal_relay->setInitContactState(SR_SELF, false);
    signal_relay->setInitContactState(SR_DLR_CTRL, true);
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
    green_relay->setInitContactState(GR_SRS_PLUS, true);

    fwd_way_relay->read_config("combine-relay");
    fwd_way_relay->setInitContactState(FWD_BUSY, false);

    allow_relay->read_config("combine-relay");
    allow_relay->setInitContactState(AR_OPEN, false);

    side_signal_relay->read_config("combine-relay");
    side_signal_relay->setInitContactState(SSR_GREEN, true);
    side_signal_relay->setInitContactState(SSR_YELLOW, false);

    connect(blink_timer, &Timer::process, this, &ExitSignal::slotBlinkTimer);
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

    side_signal_relay->step(t, dt);

    blink_timer->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::slotPressOpen()
{
    is_open_button_pressed = true;
    open_timer->start();

    Journal::instance()->info("Pressed open button for signal " + this->getLetter());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::slotPressClose()
{
    is_close_button_unpressed = false;
    close_timer->start();

    Journal::instance()->info("Pressed close button for signal " + this->getLetter());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::preStep(state_vector_t &Y, double t)
{
    lens_control();

    fwd_way_busy_control();

    removal_area_control();

    route_control();

    relay_control();
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
    old_lens_state = lens_state;

    lens_state[RED_LENS] = semaphore_signal_relay->getContactState(SRS_N_RED);

    lens_state[YELLOW_LENS] = semaphore_signal_relay->getContactState(SRS_N_YELLOW) &&
                                  (semaphore_signal_relay->getPlusContactState(SRS_PLUS_YELLOW) ||
                                  (blink_contact && side_signal_relay->getContactState(SSR_YELLOW)));

    lens_state[GREEN_LENS] = semaphore_signal_relay->getContactState(SRS_N_YELLOW) &&
                             semaphore_signal_relay->getMinusContactState(SRS_MINUS_GREEN)&&
                             side_signal_relay->getContactState(SSR_GREEN);

    if (lens_state != old_lens_state)
    {
        emit sendDataUpdate(this->serialize());
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::fwd_way_busy_control()
{
    if (conn == Q_NULLPTR)
    {
        return;
    }

    if (this->getDirection() == 1)
    {
        Trajectory *traj = conn->getFwdTraj();

        if (traj == Q_NULLPTR)
        {
            return;
        }

        is_busy = traj->isBusy();
    }

    if (this->getDirection() == -1)
    {
        Trajectory *traj = conn->getBwdTraj();

        if (traj == Q_NULLPTR)
        {
            return;
        }

        is_busy = traj->isBusy();
    }

    fwd_way_relay->setVoltage(U_bat * static_cast<double>(!is_busy));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::removal_area_control()
{
    if (conn == Q_NULLPTR)
    {
        return;
    }

    Connector *cur_conn = conn;

    bool is_GR_ON = false;
    bool is_YR_ON = false;

    // Ищем первый попутный светофор за горловиной
    while (true)
    {
        Trajectory *traj = Q_NULLPTR;

        // Берем следующую траекторию
        if (this->getDirection() == 1)
        {
            traj = cur_conn->getFwdTraj();

        }
        else
        {
            traj = cur_conn->getBwdTraj();
        }

        // Если её нет - ехать некуда
        if (traj == Q_NULLPTR)
        {
            return;
        }

        // Берем следующий коннектор
        if (this->getDirection() == 1)
        {
            cur_conn = traj->getFwdConnector();
        }
        else
        {
            cur_conn = traj->getBwdConnector();
        }

        // Если его нет - не будет никаких сигналов дальше, выходим
        if (cur_conn == Q_NULLPTR)
        {
            return;
        }

        // Проверяем, есть ли у коннектора сигнал
        Signal *signal = cur_conn->getSignalFwd();

        // если нет - это стык или стрелка, продолжаем поиск
        if (signal == Q_NULLPTR)
        {
            continue;
        }

        // Если сигнал попутный - мы у цели
        if (signal->getDirection() == this->getDirection())
        {
            // Берем коннектор сигнала
            Connector *sig_conn = signal->getConnector();

            // Страхуемся от бяки
            if (sig_conn == Q_NULLPTR)
            {
                break;
            }

            // Берем траекторию перед сигналом
            Trajectory *traj = Q_NULLPTR;

            if (this->getDirection() == 1)
            {
                traj = sig_conn->getBwdTraj();
            }
            else
            {
                traj = sig_conn->getFwdTraj();
            }

            // Если дошли сюда, траектория точно не пустая

            // Определяем занятость первого участка удаления
            is_YR_ON = !traj->isBusy();

            // Пытаемся найти второй участок удаления, беря траекторию
            // спереди от коннектора
            if (this->getDirection() == 1)
            {
                traj = sig_conn->getFwdTraj();
            }
            else
            {
                traj = sig_conn->getBwdTraj();
            }

            // Нет такой траектории, нет второго участка удаления
            if (traj == Q_NULLPTR)
            {
                break;
            }

            // Если же есть, проверяем её занятость
            is_GR_ON = !traj->isBusy();

            break;
        }
        else
        {
            break;
        }
    }

    // Отпитываем реле контроля участков приближения
    yellow_relay->setVoltage(U_bat * static_cast<double>(is_YR_ON));
    green_relay->setVoltage(U_bat * static_cast<double>(is_GR_ON));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::route_control()
{
    if (conn == Q_NULLPTR)
    {
        return;
    }

    Connector *cur_conn = conn;

    // признак свободности участков по маршруту
    bool is_free = true;
    // признак установки стрелок по маршруту
    bool is_switches_correct = true;

    // Идем по маршруту до любого ближайшего светофора
    while (true)
    {
        // Смотрим следующую за сигналом траекторию
        Trajectory *traj = Q_NULLPTR;

        if (this->getDirection() == 1)
        {
            traj = cur_conn->getFwdTraj();
        }
        else
        {
            traj = cur_conn->getBwdTraj();
        }

        // её нет - делать больше нечего
        if (traj == Q_NULLPTR)
        {
            is_free = false;
            break;
        }

        // Есть, проверяем, занята ли
        is_free = is_free && (!traj->isBusy());

        // Если занята - выходим, все уже понятно
        if (!is_free)
        {
            break;
        }

        // Берем следующий коннектор
        if (this->getDirection() == 1)
        {
            cur_conn = traj->getFwdConnector();
        }
        else
        {
            cur_conn = traj->getBwdConnector();
        }

        // Нет коннектора - ехать некуда, нет стрелки,
        // значит она - явно не по маршруту
        if (cur_conn == Q_NULLPTR)
        {
            is_switches_correct = false;
            break;
        }

        // Контроль вреза стрелки
        Trajectory *prev_traj = Q_NULLPTR;

        // Берем "заднюю" траекторию следующего коннектора
        if (this->getDirection() == 1)
        {
            prev_traj = cur_conn->getBwdTraj();
        }
        else
        {
            prev_traj = cur_conn->getFwdTraj();
        }

        // Данная проверка не бессмыслена - стрелка может стоять
        // не по маршруту и вдруг там нет траектории!
        if (prev_traj == Q_NULLPTR)
        {
            return;
        }

        // Срелка стоит по маршруту, если текущая траектория совпадает с
        // предыдущей для следующего коннектора
        is_switches_correct = is_switches_correct && (traj == prev_traj);

        // Стрелка не по маршруту? Ловить нечего - выходим из поиска
        if (!is_switches_correct)
        {
            break;
        }

        // Проверяем, дошли ли до сигнала
        Signal *signal = cur_conn->getSignalFwd();

        // Нет сигнала - продолжаем шагать
        if (signal == Q_NULLPTR)
        {
            continue;
        }

        // Дошли - выходим подбивать бабки
        break;
    }

    // Отпитываем контрольное маршрутное реле в соответсвии с проверками
    route_control_relay->setVoltage(U_bat * static_cast<double>(is_free && is_switches_correct));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::relay_control()
{
    // Цепь сигнального реле

    // Провод блока кнопок
    bool is_buttons_wire_ON = is_open_button_pressed ||
                              (signal_relay->getContactState(SR_SELF) && is_close_button_unpressed);

    bool is_SR_ON_old = is_SR_ON;

    is_SR_ON = yellow_relay->getContactState(YR_SR_CTRL) &&
                    fwd_way_relay->getContactState(FWD_BUSY) &&
                    route_control_relay->getContactState(RCR_SR_CTRL) && is_buttons_wire_ON;

    if (is_SR_ON != is_SR_ON_old)
    {
        if (is_SR_ON)
            Journal::instance()->info("Signal relay ON");
        else
            Journal::instance()->info("Signal relay OFF");
    }

    signal_relay->setVoltage(U_bat * static_cast<double>(is_SR_ON));

    // Цепь реле замыкания маршрута
    bool is_DRL_ON_old = is_DRL_ON;

    is_DRL_ON = signal_relay->getContactState(SR_DLR_CTRL);

    if (is_DRL_ON != is_DRL_ON_old)
    {
        if (is_DRL_ON)
            Journal::instance()->info("Departure lock relay ON");
        else
            Journal::instance()->info("Departure lock relay OFF");
    }

    departure_lock_relay->setVoltage(U_bat * static_cast<double>(is_DRL_ON));

    double is_minus = static_cast<double>(green_relay->getContactState(GR_SRS_MINUS));
    double is_plus = static_cast<double>(yellow_relay->getContactState(YR_SRS_PLUS) && green_relay->getContactState(GR_SRS_PLUS));

    // Напряжение питания сигнального реле светофора
    double U_srs = U_bat * (is_plus - is_minus);

    bool is_SRS_ON_old = is_SRS_ON;

    is_SRS_ON = departure_lock_relay->getContactState(DRL_LOCK) &&
                     signal_relay->getContactState(SR_SRS_CTRL) &&
                     route_control_relay->getContactState(RCR_SRS_CTRL);

    if (is_SRS_ON != is_SRS_ON_old)
    {
        if (is_SRS_ON)
            Journal::instance()->info("Semaphor signal relay ON");
        else
            Journal::instance()->info("Semaphor signal relay OFF");
    }

    semaphore_signal_relay->setVoltage(U_srs * static_cast<double>(is_SRS_ON));

    // Цепь указательного реле
    bool is_AR_ON_old = is_AR_ON;

    is_AR_ON = semaphore_signal_relay->getContactState(SRS_N_YELLOW);

    if (is_AR_ON != is_AR_ON_old)
    {
        if (is_AR_ON)
            Journal::instance()->info("Allow relay ON");
        else
            Journal::instance()->info("Allow relay OFF");
    }

    allow_relay->setVoltage(U_bat * static_cast<double>(is_AR_ON));

    // Напряжение, даваемое в линию входному
    double U_line_old = U_line_prev;

    U_dsr = U_bat * static_cast<double>(allow_relay->getContactState(AR_OPEN));

    if (qAbs(U_line_prev - U_line_old))
    {
        emit sendLineVoltage(U_line_prev);
    }

    side_signal_relay->setVoltage(U_side);

    if (side_signal_relay->getContactState(SSR_YELLOW))
    {
        blink_timer->start();
    }
    else
    {
        blink_timer->stop();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::slotOpenTimer()
{
    is_open_button_pressed = false;
    open_timer->stop();

    Journal::instance()->info("Released open button for signal " + this->getLetter());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::slotCloseTimer()
{
    is_close_button_unpressed = true;
    close_timer->stop();

    Journal::instance()->info("Released close button for signal " + this->getLetter());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::slotBlinkTimer()
{
    blink_contact = !blink_contact;
}
