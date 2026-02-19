#include    "station-signal.h"
#include    "trajectory.h"
#include    "switch.h"
#include    "Journal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
StationSignal::StationSignal(QObject* parent) : TrainSignal(parent)
{
    connect(open_timer, &Timer::process, this, &StationSignal::slotOpenTimer);
    connect(close_timer, &Timer::process, this, &StationSignal::slotCloseTimer);
    connect(blink_timer, &Timer::process, this, &StationSignal::slotBlinkTimer);
    blink_timer->start();

    call_relay->read_config("combine-relay");
    call_relay->setInitContactState(CALL_OPENED, false);
    call_relay->setInitContactState(CALL_CLOSED, true);
    call_relay->setInitContactState(CALL_SELF_CTRL, false);
    call_relay->setInitContactState(CALL_TRAIN_CTRL, true);
    call_relay->setInitContactState(CALL_SHUNT_CTRL, true);

    control_relay->read_config("combine-relay");
    control_relay->setInitContactState(CR_ALLOW_ROUTE, false);
    control_relay->setInitContactState(CR_PROHIBITED_ROUTE, true);
    control_relay->setInitContactState(CR_SIGNAL_RELAY_CTRL, false);

    signal_relay->read_config("combine-relay");
    signal_relay->setInitContactState(SR_OPENED, false);
    signal_relay->setInitContactState(SR_CLOSED, true);
    signal_relay->setInitContactState(SR_WHITE_CTRL, false);
    signal_relay->setInitContactState(SR_SELF_CTRL, false);
    signal_relay->setInitContactState(SR_CALL_CTRL, true);
    signal_relay->setInitContactState(SR_SHUNT_CTRL, true);
    signal_relay->setInitContactState(SR_LOCK_RELAY_CTRL, false);

    control_relay_shunt->read_config("combine-relay");
    control_relay_shunt->setInitContactState(CRS_ALLOW_ROUTE, false);
    control_relay_shunt->setInitContactState(CRS_PROHIBITED_ROUTE, true);
    control_relay_shunt->setInitContactState(CRS_SIGNAL_RELAY_CTRL, false);

    signal_relay_shunt->read_config("combine-relay");
    signal_relay_shunt->setInitContactState(SRS_OPENED, false);
    signal_relay_shunt->setInitContactState(SRS_CLOSED, true);
    signal_relay_shunt->setInitContactState(SRS_SELF_CTRL, false);
    signal_relay_shunt->setInitContactState(SRS_CALL_CTRL, true);
    signal_relay_shunt->setInitContactState(SRS_TRAIN_CTRL, true);
    signal_relay_shunt->setInitContactState(SRS_LOCK_ROUTE_CTRL, false);
    signal_relay_shunt->setInitContactState(SRS_LOCK_RELAY_CTRL, false);

    lock_relay->read_config("combine-relay");
    lock_relay->setInitContactState(LR_NEUTRAL_ROUTE_LOCKED, false);
    lock_relay->setInitContactState(LR_NEUTRAL_NO_ROUTE, true);
    lock_relay->setInitPlusContactState(LR_PLUS_TRAIN_LOCKED, false);
    lock_relay->setInitMinusContactState(LR_MINUS_SHUNT_LOCKED, false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
StationSignal::~StationSignal()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StationSignal::step(double t, double dt)
{
    TrainSignal::step(t, dt);

    check_train_route();

    // Цепь реле пригласительного сигнала
    // Состояние провода кнопочного блока "Открыть/Закрыть"
    bool is_CALL_ON = is_open_call_button_pressed ||
                      (is_close_button_unpressed && call_relay->getContactState(CALL_SELF_CTRL));

    is_CALL_ON &= signal_relay->getContactState(SR_CALL_CTRL) &&
                  signal_relay_shunt->getContactState(SRS_CALL_CTRL);

    call_relay->setVoltage(static_cast<double>(is_CALL_ON) * U_bat);


    // Цепь контрольного маршрутного реле
    bool is_CR_ON = call_relay->getContactState(CALL_TRAIN_CTRL) &&
                    signal_relay_shunt->getContactState(SRS_TRAIN_CTRL);

    control_relay->setVoltage(static_cast<double>(is_CR_ON) * U_way);


    // Цепь сигнального реле
    // Состояние провода кнопочного блока "Открыть/Закрыть"
    bool is_SR_ON = is_open_train_button_pressed ||
                    (is_close_button_unpressed && signal_relay->getContactState(SR_SELF_CTRL));

    // Контакт контрольного маршрутного реле
    is_SR_ON &= control_relay->getContactState(CR_SIGNAL_RELAY_CTRL);

    signal_relay->setVoltage(static_cast<double>(is_SR_ON) * U_bat);


    // Цепь контрольного реле маневрового маршрута
    bool is_CRS_ON = is_shunt_route ||
                     (is_lock_route && signal_relay_shunt->getContactState(SRS_LOCK_ROUTE_CTRL));

    is_CRS_ON &= call_relay->getContactState(CALL_SHUNT_CTRL) &&
                 signal_relay->getContactState(SR_SHUNT_CTRL);

    control_relay_shunt->setVoltage(static_cast<double>(is_CRS_ON) * U_bat);


    // Цепь сигнального реле маневрового маршрута
    // Состояние провода кнопочного блока "Открыть/Закрыть"
    bool is_SRS_ON = is_open_shunt_button_pressed ||
                     (is_close_button_unpressed && signal_relay_shunt->getContactState(SRS_SELF_CTRL));

    // Контакт контрольного реле маневрового маршрута
    is_SRS_ON &= (control_relay_shunt->getContactState(CRS_SIGNAL_RELAY_CTRL));

    signal_relay_shunt->setVoltage(static_cast<double>(is_SRS_ON) * U_bat);


    // Замыкание маршрута
    double U_LR = 0.0;
    if (signal_relay->getContactState(SR_LOCK_RELAY_CTRL))
    {
        U_LR = U_bat;
    }
    if (signal_relay_shunt->getContactState(SRS_LOCK_RELAY_CTRL))
    {
        U_LR = -U_bat;
    }
    lock_relay->setVoltage(U_LR);


    // Моделирование работы реле
    call_relay->step(t, dt);
    control_relay->step(t, dt);
    signal_relay->step(t, dt);
    control_relay_shunt->step(t, dt);
    signal_relay_shunt->step(t, dt);
    lock_relay->step(t, dt);

    // Работа таймеров удержания кнопки
    open_timer->step(t, dt);
    close_timer->step(t, dt);
    // Работа таймера мигания линз
    blink_timer->step(t, dt);

    lens_state[WHITE_LENS] = (is_next_ALSN_only && signal_relay->getContactState(SR_WHITE_CTRL)) ||
                             signal_relay_shunt->getContactState(SRS_OPENED) ||
                             (blink_contact && call_relay->getContactState(CALL_OPENED));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StationSignal::slotPressOpenTrain()
{
    is_open_train_button_pressed = true;
    is_open_shunt_button_pressed = false;
    is_open_call_button_pressed = false;
    open_timer->start();

    Journal::instance()->info("Pressed open train button for station signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StationSignal::slotPressOpenShunting()
{
    is_open_train_button_pressed = false;
    is_open_shunt_button_pressed = true;
    is_open_call_button_pressed = false;
    open_timer->start();

    Journal::instance()->info("Pressed open shunting button for station signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StationSignal::slotPressOpenCall()
{
    is_open_train_button_pressed = false;
    is_open_shunt_button_pressed = false;
    is_open_call_button_pressed = true;
    open_timer->start();

    Journal::instance()->info("Pressed open call button for station signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StationSignal::slotPressClose()
{
    is_close_button_unpressed = false;
    close_timer->start();

    Journal::instance()->info("Pressed close button for station signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StationSignal::slotOpenTimer()
{
    is_open_train_button_pressed = false;
    is_open_shunt_button_pressed = false;
    is_open_call_button_pressed = false;
    open_timer->stop();

    Journal::instance()->info("Released open buttons for station signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StationSignal::slotCloseTimer()
{
    is_close_button_unpressed = true;
    close_timer->stop();

    Journal::instance()->info("Released close button for station signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StationSignal::slotBlinkTimer()
{
    blink_contact = !blink_contact;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool StationSignal::check_and_lock_switch_fwd(Switch* sw, bool lock)
{
    // Если это не стрелка, всё хорошо и дальше делать нечего
    if (   (sw->getStateFwd() == NO_POSSIBLE_DIRECTION)
        || (sw->getStateFwd() == ONLY_MINUS)
        || (sw->getStateFwd() == ONLY_PLUS))
    {
        return true;
    }

    // Если стрелка занята подвижным составом, маршрута дальше нет
    if (sw->getStateFwd() == IS_BUSY_MINUS)
    {
        switches_state = SWITCHES_SIDE;
        return false;
    }
    if (sw->getStateFwd() == IS_BUSY_PLUS)
    {
        return false;
    }

    // Если стрелка уже занята маршрутом от другого сигнала, маршрута дальше нет
    if (sw->getRouteBySignalFwd() && (sw->getRouteBySignalFwd() != this))
    {
        return false;
    }

    // Замыкаем стрелку в маршрут
    if (lock)
    {
        if (sw->getStateFwd() < 0)
        {
            switches_state = SWITCHES_SIDE;
            sw->setRefStateFwd(IN_ROUTE_MINUS);
            sw->setRouteBySignalFwd(this);
        }
        if (sw->getStateFwd() > 0)
        {
            sw->setRefStateFwd(IN_ROUTE_PLUS);
            sw->setRouteBySignalFwd(this);
        }
        return true;
    }

    // Размыкаем маршрут
    if (sw->getRouteBySignalFwd())
    {
        if (sw->getStateFwd() < 0)
        {
            switches_state = SWITCHES_SIDE;
            sw->setRefStateFwd(STATE_MINUS);
            sw->setRouteBySignalFwd(nullptr);
        }
        if (sw->getStateFwd() > 0)
        {
            sw->setRefStateFwd(STATE_PLUS);
            sw->setRouteBySignalFwd(nullptr);
        }
    }
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool StationSignal::check_and_lock_switch_bwd(Switch* sw, bool lock)
{
    // Если это не стрелка, всё хорошо и дальше делать нечего
    if (   (sw->getStateBwd() == NO_POSSIBLE_DIRECTION)
        || (sw->getStateBwd() == ONLY_MINUS)
        || (sw->getStateBwd() == ONLY_PLUS))
    {
        return true;
    }

    // Если стрелка занята подвижным составом, маршрута дальше нет
    if (sw->getStateBwd() == IS_BUSY_MINUS)
    {
        switches_state = SWITCHES_SIDE;
        return false;
    }
    if (sw->getStateBwd() == IS_BUSY_PLUS)
    {
        return false;
    }

    // Если стрелка уже занята маршрутом от другого сигнала, маршрута дальше нет
    if (sw->getRouteBySignalBwd() && (sw->getRouteBySignalBwd() != this))
    {
        return false;
    }

    // Замыкаем стрелку в маршрут
    if (lock)
    {
        if (sw->getStateBwd() < 0)
        {
            switches_state = SWITCHES_SIDE;
            sw->setRefStateBwd(IN_ROUTE_MINUS);
            sw->setRouteBySignalBwd(this);
        }
        if (sw->getStateBwd() > 0)
        {
            sw->setRefStateBwd(IN_ROUTE_PLUS);
            sw->setRouteBySignalBwd(this);
        }
        return true;
    }

    // Размыкаем маршрут
    if (sw->getRouteBySignalBwd())
    {
        if (sw->getStateBwd() < 0)
        {
            switches_state = SWITCHES_SIDE;
            sw->setRefStateBwd(STATE_MINUS);
            sw->setRouteBySignalBwd(nullptr);
        }
        if (sw->getStateBwd() > 0)
        {
            sw->setRefStateBwd(STATE_PLUS);
            sw->setRouteBySignalBwd(nullptr);
        }
    }
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StationSignal::check_train_route()
{
    // Сбрасываем состояние
    U_way = 0.0;
    U_line = 0.0;
    U_side = 0.0;
    switches_state = SWITCHES_STRAIGHT;
    is_next_ALSN_only = false;
    is_shunt_route = false;
    is_lock_route = true;

    // Признак блокирования траекторий и стрелок в маршрут
    bool lock = lock_relay->getContactState(LR_NEUTRAL_ROUTE_LOCKED);

    // Начинаем с коннектора, к которому относится светофор
    Switch* cur_conn = conn;

    if (!cur_conn)
    {
        return;
    }

    // Смотрим траекторию перед текущим коннектором (участок приближения)
    dir_t cur_dir = static_cast<dir_t>(-signal_dir);
    Trajectory* traj = cur_conn->getNextTraj(cur_dir);
    if (traj && !traj->isBusy())
    {
        // Если траектория свободна, разрешаем размыкание маневрого маршрута
        is_lock_route = false;
    }

    // Смотрим траекторию за текущим коннектором
    cur_dir = signal_dir;
    traj = cur_conn->getNextTraj(cur_dir);
    if (traj && ((traj == ref_trajectory_shunt) || !traj->isBusy()))
    {
        // Если траектория свободна, разрешаем размыкание маневрого маршрута
        is_lock_route = false;
    }

    // Блокировка противошёрстного стрелочного перевода за светофором в маршрут
    if (signal_dir == 1)
    {
        if (!check_and_lock_switch_fwd(cur_conn, lock))
        {
            return;
        }
    }
    else
    {
        if (!check_and_lock_switch_bwd(cur_conn, lock))
        {
            return;
        }
    }

    cur_dir = signal_dir;
    while (true)
    {
        // Смотрим траекторию за текущим коннектором
        traj = cur_conn->getNextTraj(cur_dir);

        // Уперлись в тупик - разрешаем маневровый маршрут до тупика
        if (!traj)
        {
            is_shunt_route = true;
            return;
        }

        // Проверяем включение траектории в маршрут от другого светофора
        if (traj->isInRoute())
        {
            Signal* s = (cur_dir == FWD) ? traj->getRouteBySignalFwd() : traj->getRouteBySignalBwd();
            if (s != this)
            {
                return;
            }
        }

        // Нашли целевую траекторию
        if (traj == ref_trajectory_shunt)
        {
            // Радостно включаем реле контроля маневрового маршрута
            is_shunt_route = true;
        }

        // Проверяем занятость траектории
        if (traj->isBusy())
        {
            return;
        }

        // Занимаем траекторию маршрутом от данного светофора
        lock = (lock_relay->getContactState(LR_PLUS_TRAIN_LOCKED) ||
                (!is_shunt_route && lock_relay->getContactState(LR_MINUS_SHUNT_LOCKED)));
        if (lock)
        {
            traj->setInRoute(true);
            (cur_dir == FWD) ? traj->setRouteBySignalFwd(this) : traj->setRouteBySignalBwd(this);
        }
        else
        {
            traj->setInRoute(false);
            (cur_dir == FWD) ? traj->setRouteBySignalFwd(nullptr) : traj->setRouteBySignalBwd(nullptr);
        }

        // Смотрим следующий коннектор
        cur_conn = traj->getNextSwitch(cur_dir);

        // Уперлись в тупик - разрешаем маневровый маршрут до тупика
        if (!cur_conn)
        {
            is_shunt_route = true;
            return;
        }

        // Контроль взреза стрелки: смотрим траекторию перед следующим коннектором
        dir_t prev_dir = static_cast<dir_t>(-cur_dir);
        Trajectory* prev = cur_conn->getNextTraj(prev_dir);

        if (traj != prev)
        {
            return;
        }

        // Смотрим сигнал на следующем коннекторе
        Signal* signal = (cur_dir == FWD) ? cur_conn->getSignalFwd() : cur_conn->getSignalBwd();

        if (signal)
        {
            // Блокировка пошёрстного стрелочного перевода перед светофором в маршрут
            if (cur_dir == FWD)
            {
                if (!check_and_lock_switch_bwd(cur_conn, lock))
                {
                    return;
                }
            }
            else
            {
                if (!check_and_lock_switch_fwd(cur_conn, lock))
                {
                    return;
                }
            }

            // Если нашли маршрут до следующего поездного светофора, заканчиваем цикл
            if (TrainSignal* ts = dynamic_cast<TrainSignal*>(signal))
            {
                // Запоминаем, если следующая сигнальная точка без напольного светофора
                is_next_ALSN_only = (ts->getSignalModel().left(5) == "empty");

                // Если сигнал закрыт, отключаем АЛСН-код от следующего светофора
                ts->allowTransmitALSN(!lens_state[RED_LENS]);

                // Замыкание цепи на путевое реле в релейном шкафу следующего светофора
                U_way = U_bat;
                // Линейное реле питается от линии следующего светофора
                U_line = ts->getLineVoltage();
                // Боковое сигнальное реле питается от линии следующего светофора
                U_side = ts->getSideVoltage();
                // Радостно включаем реле контроля маневрого маршрута и заканчиваем
                is_shunt_route = true;
                return;
            }

            // Любой встреченный светофор заканчивает маневровый маршрут
            is_shunt_route = true;

            // Продолжаем обход топологии для поездного маршрута,
            // замыкаем противошёрстный стрелочный перевод
            lock = lock_relay->getContactState(LR_PLUS_TRAIN_LOCKED);
            if (cur_dir == FWD)
            {
                if (!check_and_lock_switch_fwd(cur_conn, lock))
                {
                    return;
                }
            }
            else
            {
                if (!check_and_lock_switch_bwd(cur_conn, lock))
                {
                    return;
                }
            }
        }
        else
        {
            // Блокировка стрелочных переводов в маршрут
            if (!check_and_lock_switch_bwd(cur_conn, lock) ||
                !check_and_lock_switch_fwd(cur_conn, lock))
            {
                return;
            }
        }
    }
}
