#include    "shunting-signal.h"
#include    "enter-signal.h"
#include    "trajectory.h"
#include    "switch.h"
#include    "Journal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ShuntingSignal::ShuntingSignal(QObject* parent) : Signal(parent)
{
    connect(open_timer, &Timer::process, this, &ShuntingSignal::slotOpenTimer);
    connect(close_timer, &Timer::process, this, &ShuntingSignal::slotCloseTimer);

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
    control_relay->setVoltage(U_ctrl);


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
void ShuntingSignal::slotPressOpen()
{
    is_open_button_pressed = true;
    open_timer->start();

    Journal::instance()->info("Pressed open button for station signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ShuntingSignal::slotPressClose()
{
    is_close_button_unpressed = false;
    close_timer->start();

    Journal::instance()->info("Pressed close button for station signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ShuntingSignal::slotOpenTimer()
{
    is_open_button_pressed = false;
    open_timer->stop();

    Journal::instance()->info("Released open button for station signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ShuntingSignal::slotCloseTimer()
{
    is_close_button_unpressed = true;
    close_timer->stop();

    Journal::instance()->info("Released close button for station signal " + letter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ShuntingSignal::check_shunt_route()
{
    // Сбрасываем состояние
    U_ctrl = 0.0;

    // Начинаем с коннектора, к которому относится светофор
    Connector *cur_conn = conn;

    if (!cur_conn)
    {
        return;
    }

    while (true)
    {
        // Смотрим траекторию за текущим коннектором
        Trajectory* traj = (signal_dir == 1) ? cur_conn->getFwdTraj() : cur_conn->getBwdTraj();

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
        if (traj == ref_trajectory)
        {
            U_ctrl = U_bat;
            return;
        }

        // Проверяем занятость траектории
        if (traj->isBusy())
        {
            return;
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

        // Смотрим стрелочный перевод на коннекторе
        if (Switch* sw = dynamic_cast<Switch*>(cur_conn))
        {
            // Блокировка стрелочных переводов в маршруте
            if (lock_relay->getContactState(LR_ROUTE_LOCKED))
            {
                if (sw->getStateBwd() < 0)
                {
                    sw->setRefStateBwd(Switch::IN_ROUTE_MINUS);
                }
                if (sw->getStateBwd() > 0)
                {
                    sw->setRefStateBwd(Switch::IN_ROUTE_PLUS);
                }
                if (sw->getStateFwd() < 0)
                {
                    sw->setRefStateFwd(Switch::IN_ROUTE_MINUS);
                }
                if (sw->getStateFwd() > 0)
                {
                    sw->setRefStateFwd(Switch::IN_ROUTE_PLUS);
                }
            }
            else
            {
                if (sw->getStateBwd() < 0)
                {
                    sw->setRefStateBwd(Switch::STATE_MINUS);
                }
                if (sw->getStateBwd() > 0)
                {
                    sw->setRefStateBwd(Switch::STATE_PLUS);
                }
                if (sw->getStateFwd() < 0)
                {
                    sw->setRefStateFwd(Switch::STATE_MINUS);
                }
                if (sw->getStateFwd() > 0)
                {
                    sw->setRefStateFwd(Switch::STATE_PLUS);
                }
            }
        }

        // Смотрим сигнал на следующем коннекторе
        Signal* signal = (signal_dir == 1) ? cur_conn->getSignalFwd() : cur_conn->getSignalBwd();

        if (signal)
        {
            U_ctrl = U_bat;
            return;
        }
    }
}
