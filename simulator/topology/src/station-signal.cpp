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

    control_relay->read_config("combine-relay");
    control_relay->setInitContactState(CR_ALLOW_ROUTE, false);
    control_relay->setInitContactState(CR_PROHIBITED_ROUTE, true);
    control_relay->setInitContactState(CR_SIGNAL_RELAY_CTRL, false);

    signal_relay->read_config("combine-relay");
    signal_relay->setInitContactState(SR_OPENED, false);
    signal_relay->setInitContactState(SR_CLOSED, true);
    signal_relay->setInitContactState(SR_SELF_CTRL, false);
    signal_relay->setInitContactState(SR_LOCK_RELAY_CTRL, true);

    lock_relay->read_config("combine-relay");
    lock_relay->setInitContactState(LR_ROUTE_LOCKED, true);
    lock_relay->setInitContactState(LR_NO_ROUTE, false);
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


    // Цепь контрольного маршрутного реле
    control_relay->setVoltage(U_way);


    // Цепь сигнального реле
    // Состояние провода кнопочного блока "Открыть/Закрыть"
    bool is_SR_ON = is_open_button_pressed ||
                    (is_close_button_unpressed && signal_relay->getContactState(SR_SELF_CTRL));

    // Контакт контрольного маршрутного реле
    is_SR_ON &= control_relay->getContactState(CR_SIGNAL_RELAY_CTRL);

    signal_relay->setVoltage(static_cast<double>(is_SR_ON) * U_bat);


    // Замыкание маршрута
    bool is_LR_ON = signal_relay->getContactState(SR_LOCK_RELAY_CTRL);

    lock_relay->setVoltage(static_cast<double>(is_LR_ON) * U_bat);


    // Моделирование работы реле
    control_relay->step(t, dt);
    signal_relay->step(t, dt);
    lock_relay->step(t, dt);

    // Работа таймеров удержания кнопки
    open_timer->step(t, dt);
    close_timer->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StationSignal::slotPressOpen()
{
    is_open_button_pressed = true;
    open_timer->start();

    Journal::instance()->info("Pressed open button for station signal " + letter);
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
    is_open_button_pressed = false;
    open_timer->stop();

    Journal::instance()->info("Released open button for station signal " + letter);
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

    // Начинаем с коннектора, к которому относится светофор
    Connector *cur_conn = conn;

    if (!cur_conn)
    {
        return;
    }

    // Смотрим стрелочный перевод на текущем коннекторе
    if (Switch* sw = dynamic_cast<Switch*>(cur_conn))
    {
        // Блокировка противошёрстного стрелочного перевода за светофором в маршрут
        if (signal_dir == 1)
        {
            if (!check_and_lock_switch_fwd(sw, lock_relay->getContactState(LR_ROUTE_LOCKED)))
            {
                return;
            }
        }
        else
        {
            if (!check_and_lock_switch_bwd(sw, lock_relay->getContactState(LR_ROUTE_LOCKED)))
            {
                return;
            }
        }
    }

    while (true)
    {
        // Смотрим траекторию за текущим коннектором
        Trajectory* traj = (signal_dir == 1) ? cur_conn->getFwdTraj() : cur_conn->getBwdTraj();

        if (!traj)
        {
            return;
        }

        // Проверяем занятость траектории
        if (traj->isBusy())
        {
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

        // Занимаем траекторию маршрутом от данного светофора
        if (lock_relay->getContactState(LR_ROUTE_LOCKED))
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

        if (!cur_conn)
        {
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
            // Нашли маршрут до следующего поездного светофора, заканчиваем цикл
            if (TrainSignal* ts = dynamic_cast<TrainSignal*>(signal))
            {
                // Смотрим стрелочный перевод на коннекторе
                if (Switch* sw = dynamic_cast<Switch*>(cur_conn))
                {
                    // Блокировка пошёрстного стрелочного перевода перед светофором в маршрут
                    if (signal_dir == 1)
                    {
                        if (!check_and_lock_switch_bwd(sw, lock_relay->getContactState(LR_ROUTE_LOCKED)))
                        {
                            return;
                        }
                    }
                    else
                    {
                        if (!check_and_lock_switch_fwd(sw, lock_relay->getContactState(LR_ROUTE_LOCKED)))
                        {
                            return;
                        }
                    }
                }

                // Если сигнал закрыт, отключаем АЛСН-код от следующего светофора
                ts->allowTransmitALSN(!lens_state[RED_LENS]);

                // Замыкание цепи на путевое реле в релейном шкафу следующего светофора
                U_way = U_bat;
                // Линейное реле питается от линии следующего светофора
                U_line = ts->getLineVoltage();
                // Боковое сигнальное реле питается от линии следующего светофора
                U_side = ts->getSideVoltage();
                return;
            }
        }

        // Смотрим стрелочный перевод на коннекторе
        if (Switch* sw = dynamic_cast<Switch*>(cur_conn))
        {
            // Блокировка стрелочных переводов в маршрут
            if (!check_and_lock_switch_bwd(sw, lock_relay->getContactState(LR_ROUTE_LOCKED)) ||
                !check_and_lock_switch_fwd(sw, lock_relay->getContactState(LR_ROUTE_LOCKED)))
            {
                return;
            }
        }
    }
}
