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

    lens_state[WHITE_LENS] = signal_relay_shunt->getContactState(SRS_OPENED) ||
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
    if (sw->getStateFwd() == Switch::ONE_POSSIBLE_DIRECTION)
    {
        return true;
    }

    // Если стрелка занята подвижным составом, маршрута дальше нет
    if (sw->getStateFwd() == Switch::IS_BUSY_MINUS)
    {
        switches_state = SWITCHES_SIDE;
        return false;
    }
    if (sw->getStateFwd() == Switch::IS_BUSY_PLUS)
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
            sw->setRefStateFwd(Switch::IN_ROUTE_MINUS);
            sw->setRouteBySignalFwd(this);
        }
        if (sw->getStateFwd() > 0)
        {
            sw->setRefStateFwd(Switch::IN_ROUTE_PLUS);
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
            sw->setRefStateFwd(Switch::STATE_MINUS);
            sw->setRouteBySignalFwd(nullptr);
        }
        if (sw->getStateFwd() > 0)
        {
            sw->setRefStateFwd(Switch::STATE_PLUS);
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
    if (sw->getStateBwd() == Switch::ONE_POSSIBLE_DIRECTION)
    {
        return true;
    }

    // Если стрелка занята подвижным составом, маршрута дальше нет
    if (sw->getStateBwd() == Switch::IS_BUSY_MINUS)
    {
        switches_state = SWITCHES_SIDE;
        return false;
    }
    if (sw->getStateBwd() == Switch::IS_BUSY_PLUS)
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
            sw->setRefStateBwd(Switch::IN_ROUTE_MINUS);
            sw->setRouteBySignalBwd(this);
        }
        if (sw->getStateBwd() > 0)
        {
            sw->setRefStateBwd(Switch::IN_ROUTE_PLUS);
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
            sw->setRefStateBwd(Switch::STATE_MINUS);
            sw->setRouteBySignalBwd(nullptr);
        }
        if (sw->getStateBwd() > 0)
        {
            sw->setRefStateBwd(Switch::STATE_PLUS);
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
    is_shunt_route = false;
    is_lock_route = true;

    // Признак блокирования траекторий и стрелок в маршрут
    bool lock = lock_relay->getContactState(LR_NEUTRAL_ROUTE_LOCKED);

    // Начинаем с коннектора, к которому относится светофор
    Connector *cur_conn = conn;

    if (!cur_conn)
    {
        return;
    }

    // Смотрим траекторию перед текущим коннектором (участок приближения)
    Trajectory* traj = (signal_dir == 1) ? cur_conn->getBwdTraj() : cur_conn->getFwdTraj();
    if (traj && !traj->isBusy())
    {
        // Если траектория свободна, разрешаем размыкание маневрого маршрута
        is_lock_route = false;
    }

    // Смотрим траекторию за текущим коннектором
    traj = (signal_dir == 1) ? cur_conn->getFwdTraj() : cur_conn->getBwdTraj();
    if (traj && ((traj == ref_trajectory_shunt) || !traj->isBusy()))
    {
        // Если траектория свободна, разрешаем размыкание маневрого маршрута
        is_lock_route = false;
    }

    // Смотрим стрелочный перевод на текущем коннекторе
    if (Switch* sw = dynamic_cast<Switch*>(cur_conn))
    {
        // Блокировка противошёрстного стрелочного перевода за светофором в маршрут
        if (signal_dir == 1)
        {
            if (!check_and_lock_switch_fwd(sw, lock))
            {
                return;
            }
        }
        else
        {
            if (!check_and_lock_switch_bwd(sw, lock))
            {
                return;
            }
        }
    }

    while (true)
    {
        // Смотрим траекторию за текущим коннектором
        traj = (signal_dir == 1) ? cur_conn->getFwdTraj() : cur_conn->getBwdTraj();

        // Уперлись в тупик - разрешаем маневровый маршрут до тупика
        if (!traj)
        {
            is_shunt_route = true;
            return;
        }

        // Проверяем включение траектории в маршрут от другого светофора
        if (traj->isInRoute())
        {
            Signal* s = (signal_dir == 1) ? traj->getRouteBySignalFwd() : traj->getRouteBySignalBwd();
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
            (signal_dir == 1) ? traj->setRouteBySignalFwd(this) : traj->setRouteBySignalBwd(this);
        }
        else
        {
            traj->setInRoute(false);
            (signal_dir == 1) ? traj->setRouteBySignalFwd(nullptr) : traj->setRouteBySignalBwd(nullptr);
        }

        // Смотрим следующий коннектор
        cur_conn = (signal_dir == 1) ? traj->getFwdConnector() : traj->getBwdConnector();

        // Уперлись в тупик - разрешаем маневровый маршрут до тупика
        if (!cur_conn)
        {
            is_shunt_route = true;
            return;
        }

        // Контроль взреза стрелки: смотрим траекторию перед следующим коннектором
        Trajectory* prev = (signal_dir == 1) ? cur_conn->getBwdTraj() : cur_conn->getFwdTraj();

        if (traj != prev)
        {
            return;
        }

        // Смотрим сигнал на следующем коннекторе
        Signal* signal = (signal_dir == 1) ? cur_conn->getSignalFwd() : cur_conn->getSignalBwd();

        if (signal)
        {
            // Смотрим стрелочный перевод на коннекторе
            if (Switch* sw = dynamic_cast<Switch*>(cur_conn))
            {
                // Блокировка пошёрстного стрелочного перевода перед светофором в маршрут
                if (signal_dir == 1)
                {
                    if (!check_and_lock_switch_bwd(sw, lock))
                    {
                        return;
                    }
                }
                else
                {
                    if (!check_and_lock_switch_fwd(sw, lock))
                    {
                        return;
                    }
                }
            }

            // Если нашли маршрут до следующего поездного светофора, заканчиваем цикл
            if (TrainSignal* ts = dynamic_cast<TrainSignal*>(signal))
            {

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
            if (Switch* sw = dynamic_cast<Switch*>(cur_conn))
            {
                lock = lock_relay->getContactState(LR_PLUS_TRAIN_LOCKED);
                if (signal_dir == 1)
                {
                    if (!check_and_lock_switch_fwd(sw, lock))
                    {
                        return;
                    }
                }
                else
                {
                    if (!check_and_lock_switch_bwd(sw, lock))
                    {
                        return;
                    }
                }
            }
        }
        else
        {
            // Смотрим стрелочный перевод на коннекторе
            if (Switch* sw = dynamic_cast<Switch*>(cur_conn))
            {
                // Блокировка стрелочных переводов в маршрут
                if (!check_and_lock_switch_bwd(sw, lock) ||
                    !check_and_lock_switch_fwd(sw, lock))
                {
                    return;
                }
            }
        }
    }
}
