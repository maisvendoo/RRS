#include    "shunting-signal.h"
#include    "trajectory.h"
#include    "switch.h"
#include    "Journal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ShuntingSignal::ShuntingSignal(QObject* parent) : Signal(parent)
{
    connect(open_timer, &Timer::process, this, &ShuntingSignal::slotOpenTimerShunt);
    connect(close_timer, &Timer::process, this, &ShuntingSignal::slotCloseTimer);

    control_relay_shunt->read_config("combine-relay");
    control_relay_shunt->setInitContactState(CRS_ALLOW_ROUTE, false);
    control_relay_shunt->setInitContactState(CRS_PROHIBITED_ROUTE, true);
    control_relay_shunt->setInitContactState(CRS_SIGNAL_RELAY_CTRL, false);

    signal_relay_shunt->read_config("combine-relay");
    signal_relay_shunt->setInitContactState(SRS_OPENED, false);
    signal_relay_shunt->setInitContactState(SRS_CLOSED, true);
    signal_relay_shunt->setInitContactState(SRS_SELF_CTRL, false);
    signal_relay_shunt->setInitContactState(SRS_LOCK_RELAY_CTRL, true);
    signal_relay_shunt->setInitContactState(SRS_UNLOCK_RELAY_CTRL, true);

    lock_relay_shunt->read_config("combine-relay");
    lock_relay_shunt->setInitContactState(LRS_ROUTE_LOCKED, true);
    lock_relay_shunt->setInitContactState(LRS_NO_ROUTE, false);

    unlock_relay_shunt->read_config("combine-relay");
    unlock_relay_shunt->setInitContactState(URS_ROUTE_LOCKED, true);
    unlock_relay_shunt->setInitContactState(URS_ROUTE_UNLOCKED, false);
    unlock_relay_shunt->setInitContactState(URS_SIGNAL_RELAY_CTRL, true);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ShuntingSignal::~ShuntingSignal()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ShuntingSignal::step(double t, double dt)
{
    Signal::step(t, dt);

    check_shunt_route();

    // Цепь контрольного маршрутного реле
    control_relay_shunt->setVoltage(U_ctrl_shunt);


    // Цепь сигнального реле
    // Состояние провода кнопочного блока "Открыть/Закрыть"
    bool is_SRS_ON = is_open_shunt_button_pressed ||
                    (is_close_button_unpressed && signal_relay_shunt->getContactState(SRS_SELF_CTRL));

    // Контакты контрольного маршрутного реле или реле размыкания маршрута
    is_SRS_ON &= (control_relay_shunt->getContactState(CRS_SIGNAL_RELAY_CTRL) ||
                  unlock_relay_shunt->getContactState(URS_SIGNAL_RELAY_CTRL));

    signal_relay_shunt->setVoltage(static_cast<double>(is_SRS_ON) * U_bat);


    // Замыкание маршрута
    bool is_LRS_ON = signal_relay_shunt->getContactState(SRS_LOCK_RELAY_CTRL);

    lock_relay_shunt->setVoltage(static_cast<double>(is_LRS_ON) * U_bat);


    // Размыкание маршрута
    bool is_URS_ON = signal_relay_shunt->getContactState(SRS_UNLOCK_RELAY_CTRL);

    unlock_relay_shunt->setVoltage(static_cast<double>(is_URS_ON) * U_unlock_shunt);


    // Моделирование работы реле
    control_relay_shunt->step(t, dt);
    signal_relay_shunt->step(t, dt);
    lock_relay_shunt->step(t, dt);
    unlock_relay_shunt->step(t, dt);

    // Работа таймеров удержания кнопки
    open_timer->step(t, dt);
    close_timer->step(t, dt);

    lens_state[BLUE_LENS] = signal_relay_shunt->getContactState(SRS_CLOSED);
    lens_state[WHITE_LENS] = signal_relay_shunt->getContactState(SRS_OPENED);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ShuntingSignal::slotPressOpenShunt()
{
    is_open_shunt_button_pressed = true;
    open_timer->start();

    Journal::instance()->info("Pressed open button for shunting signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ShuntingSignal::slotPressClose()
{
    is_close_button_unpressed = false;
    close_timer->start();

    Journal::instance()->info("Pressed close button for shunting signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ShuntingSignal::slotOpenTimerShunt()
{
    is_open_shunt_button_pressed = false;
    open_timer->stop();

    Journal::instance()->info("Released open button for shunting signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ShuntingSignal::slotCloseTimer()
{
    is_close_button_unpressed = true;
    close_timer->stop();

    Journal::instance()->info("Released close button for shunting signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ShuntingSignal::check_and_lock_switch_fwd(Switch* sw, bool lock)
{
    // Если это не стрелка, всё хорошо и дальше делать нечего
    if (sw->getStateFwd() == Switch::ONE_POSSIBLE_DIRECTION)
    {
        return true;
    }

    // Если стрелка занята подвижным составом, маршрута дальше нет
    if ((sw->getStateFwd() == Switch::IS_BUSY_MINUS) ||
        (sw->getStateFwd() == Switch::IS_BUSY_PLUS))
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
bool ShuntingSignal::check_and_lock_switch_bwd(Switch* sw, bool lock)
{
    // Если это не стрелка, всё хорошо и дальше делать нечего
    if (sw->getStateBwd() == Switch::ONE_POSSIBLE_DIRECTION)
    {
        return true;
    }

    // Если стрелка занята подвижным составом, маршрута дальше нет
    if ((sw->getStateBwd() == Switch::IS_BUSY_MINUS) ||
        (sw->getStateBwd() == Switch::IS_BUSY_PLUS))
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
void ShuntingSignal::check_shunt_route()
{
    // Сбрасываем состояние
    U_ctrl_shunt = 0.0;
    U_unlock_shunt = 0.0;

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
        U_unlock_shunt = U_bat;
    }

    // Смотрим траекторию за текущим коннектором
    traj = (signal_dir == 1) ? cur_conn->getFwdTraj() : cur_conn->getBwdTraj();
    if (traj && ((traj == ref_trajectory_shunt) || !traj->isBusy()))
    {
        // Если траектория свободна, разрешаем размыкание маневрого маршрута
        U_unlock_shunt = U_bat;
    }

    // Смотрим стрелочный перевод на текущем коннекторе
    if (Switch* sw = dynamic_cast<Switch*>(cur_conn))
    {
        // Блокировка противошёрстного стрелочного перевода за светофором в маршрут
        if (signal_dir == 1)
        {
            if (!check_and_lock_switch_fwd(sw, lock_relay_shunt->getContactState(LRS_ROUTE_LOCKED)))
            {
                return;
            }
        }
        else
        {
            if (!check_and_lock_switch_bwd(sw, lock_relay_shunt->getContactState(LRS_ROUTE_LOCKED)))
            {
                return;
            }
        }
    }

    while (true)
    {
        // Смотрим траекторию за текущим коннектором
        traj = (signal_dir == 1) ? cur_conn->getFwdTraj() : cur_conn->getBwdTraj();

        if (!traj)
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

        // Нашли целевую траекторию
        if (traj == ref_trajectory_shunt)
        {
            // Радостно включаем реле контроля маршрута и заканчиваем
            U_ctrl_shunt = U_bat;
            return;
        }

        // Проверяем занятость траектории
        if (traj->isBusy())
        {
            return;
        }

        // Занимаем траекторию маршрутом от данного светофора
        if (lock_relay_shunt->getContactState(LRS_ROUTE_LOCKED))
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

        // Нашли маршрут до следующего светофора, заканчиваем цикл
        if (signal)
        {
            // Смотрим стрелочный перевод на коннекторе
            if (Switch* sw = dynamic_cast<Switch*>(cur_conn))
            {
                // Блокировка пошёрстного стрелочного перевода перед светофором в маршрут
                if (signal_dir == 1)
                {
                    if (!check_and_lock_switch_bwd(sw, lock_relay_shunt->getContactState(LRS_ROUTE_LOCKED)))
                    {
                        return;
                    }
                }
                else
                {
                    if (!check_and_lock_switch_fwd(sw, lock_relay_shunt->getContactState(LRS_ROUTE_LOCKED)))
                    {
                        return;
                    }
                }
            }

            // Радостно включаем реле контроля маршрута и заканчиваем
            U_ctrl_shunt = U_bat;
            return;
        }

        // Смотрим стрелочный перевод на коннекторе
        if (Switch* sw = dynamic_cast<Switch*>(cur_conn))
        {
            // Блокировка стрелочных переводов в маршрут
            if (!check_and_lock_switch_bwd(sw, lock_relay_shunt->getContactState(LRS_ROUTE_LOCKED)) ||
                !check_and_lock_switch_fwd(sw, lock_relay_shunt->getContactState(LRS_ROUTE_LOCKED)))
            {
                return;
            }
        }
    }
}
