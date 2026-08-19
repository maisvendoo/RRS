#include    "shunting-signal.h"

#include    "Journal.h"
#include    "relay.h"
#include    "switch.h"
#include    "timer.h"
#include    "trajectory.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ShuntingSignal::ShuntingSignal(QObject* parent) : Signal(parent)
{
    control_relay_shunt = new Relay(NUM_CRS_CONTACTS);
    signal_relay_shunt = new Relay(NUM_SRS_CONTACTS);
    lock_relay_shunt = new Relay(NUM_LRS_CONTACTS);

    open_timer = new Timer(1.0, false);
    close_timer = new Timer(1.0, false);

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
    signal_relay_shunt->setInitContactState(SRS_LOCK_ROUTE_CTRL, false);
    signal_relay_shunt->setInitContactState(SRS_LOCK_RELAY_CTRL, true);

    lock_relay_shunt->read_config("combine-relay");
    lock_relay_shunt->setInitContactState(LRS_ROUTE_LOCKED, true);
    lock_relay_shunt->setInitContactState(LRS_NO_ROUTE, false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ShuntingSignal::~ShuntingSignal() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ShuntingSignal::step(double t, double dt)
{
    Signal::step(t, dt);

    check_shunt_route();

    // Цепь контрольного маршрутного реле
    bool is_CRS_ON = is_shunt_route ||
                     (is_lock_route && signal_relay_shunt->getContactState(SRS_LOCK_ROUTE_CTRL));

    control_relay_shunt->setVoltage(static_cast<double>(is_CRS_ON) * U_bat);


    // Цепь сигнального реле
    // Состояние провода кнопочного блока "Открыть/Закрыть"
    bool is_SRS_ON = is_open_shunt_button_pressed ||
                    (is_close_button_unpressed && signal_relay_shunt->getContactState(SRS_SELF_CTRL));

    // Контакт контрольного маршрутного реле
    is_SRS_ON &= control_relay_shunt->getContactState(CRS_SIGNAL_RELAY_CTRL);

    signal_relay_shunt->setVoltage(static_cast<double>(is_SRS_ON) * U_bat);


    // Замыкание маршрута
    bool is_LRS_ON = signal_relay_shunt->getContactState(SRS_LOCK_RELAY_CTRL);

    lock_relay_shunt->setVoltage(static_cast<double>(is_LRS_ON) * U_bat);


    // Моделирование работы реле
    control_relay_shunt->step(t, dt);
    signal_relay_shunt->step(t, dt);
    lock_relay_shunt->step(t, dt);

    // Работа таймеров удержания кнопки
    open_timer->step(t, dt);
    close_timer->step(t, dt);

    lens_state[BLUE_LENS] = signal_relay_shunt->getContactState(SRS_CLOSED);
    lens_state[WHITE_LENS] = signal_relay_shunt->getContactState(SRS_OPENED);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ShuntingSignal::slotPressOpenShunting()
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
    if (   (sw->getStateFwd() == NO_POSSIBLE_DIRECTION)
        || (sw->getStateFwd() == ONLY_MINUS)
        || (sw->getStateFwd() == ONLY_PLUS))
    {
        return true;
    }

    // Если стрелка занята подвижным составом, маршрута дальше нет
    if ((sw->getStateFwd() == IS_BUSY_MINUS) ||
        (sw->getStateFwd() == IS_BUSY_PLUS))
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

    // Иначе, размыкаем маршрут
    if (sw->getRouteBySignalFwd())
    {
        if (sw->getStateFwd() < 0)
        {
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
bool ShuntingSignal::check_and_lock_switch_bwd(Switch* sw, bool lock)
{
    // Если это не стрелка, всё хорошо и дальше делать нечего
    if (   (sw->getStateBwd() == NO_POSSIBLE_DIRECTION)
        || (sw->getStateBwd() == ONLY_MINUS)
        || (sw->getStateBwd() == ONLY_PLUS))
    {
        return true;
    }

    // Если стрелка занята подвижным составом, маршрута дальше нет
    if ((sw->getStateBwd() == IS_BUSY_MINUS) ||
        (sw->getStateBwd() == IS_BUSY_PLUS))
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

    // Иначе, размыкаем маршрут
    if (sw->getRouteBySignalBwd())
    {
        if (sw->getStateBwd() < 0)
        {
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
void ShuntingSignal::check_shunt_route()
{
    // Сбрасываем состояние
    is_shunt_route = false;
    is_lock_route = true;

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
    if (traj && !traj->isBusy())
    {
        // Если траектория свободна, разрешаем размыкание маневрого маршрута
        is_lock_route = false;
    }

    // Блокировка противошёрстного стрелочного перевода за светофором в маршрут
    if (signal_dir == 1)
    {
        if (!check_and_lock_switch_fwd(cur_conn, lock_relay_shunt->getContactState(LRS_ROUTE_LOCKED)))
        {
            return;
        }
    }
    else
    {
        if (!check_and_lock_switch_bwd(cur_conn, lock_relay_shunt->getContactState(LRS_ROUTE_LOCKED)))
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

        // Проверяем занятость траектории
        if (traj->isBusy())
        {
            // Разрешаем маневровый маршрут до занятой траектории
            is_shunt_route = true;
            return;
        }

        // Занимаем траекторию маршрутом от данного светофора
        if (lock_relay_shunt->getContactState(LRS_ROUTE_LOCKED))
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

        // Нашли маршрут до следующего светофора, заканчиваем цикл
        if (signal)
        {
            // Блокировка пошёрстного стрелочного перевода перед светофором в маршрут
            if (cur_dir == FWD)
            {
                if (!check_and_lock_switch_bwd(cur_conn, lock_relay_shunt->getContactState(LRS_ROUTE_LOCKED)))
                {
                    return;
                }
            }
            else
            {
                if (!check_and_lock_switch_fwd(cur_conn, lock_relay_shunt->getContactState(LRS_ROUTE_LOCKED)))
                {
                    return;
                }
            }

            // Радостно включаем реле контроля маршрута и заканчиваем
            is_shunt_route = true;
            return;
        }

        // Блокировка стрелочных переводов в маршрут
        if (!check_and_lock_switch_bwd(cur_conn, lock_relay_shunt->getContactState(LRS_ROUTE_LOCKED)) ||
            !check_and_lock_switch_fwd(cur_conn, lock_relay_shunt->getContactState(LRS_ROUTE_LOCKED)))
        {
            return;
        }
    }
}
