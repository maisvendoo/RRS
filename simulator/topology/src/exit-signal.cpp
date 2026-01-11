#include    <exit-signal.h>
#include    <switch.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ExitSignal::ExitSignal(QObject *parent) : StationSignal(parent)
{
    connect(blink_timer, &Timer::process, this, &ExitSignal::slotBlinkTimer);

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
    StationSignal::step(t, dt);

    blink_timer->step(t, dt);

    semaphore_signal_relay->step(t, dt);
    side_signal_relay->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::preStep(double t)
{
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

        // Проверяем занятость траектории
        if (traj->isBusy())
        {
            U_way = 0.0;
            U_line = 0.0;
            U_side = 0.0;
            return;
        }

        // Проверяем включение траектории в маршрут от другого светофора
        if (traj->isInRoute())
        {
            Signal* s = (signal_dir == 1) ? traj->getRouteBySignalFwd() : traj->getRouteBySignalBwd();
            if (s != this)
            {
                U_way = 0.0;
                U_line = 0.0;
                U_side = 0.0;
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
            if (TrainSignal* ts = dynamic_cast<TrainSignal*>(signal))
            {
                // Замыкание цепи на путевое реле в релейном шкафу следующего светофора
                U_way = U_bat;
                // Линейное реле питается от линии следующего светофора
                U_line = ts->getLineVoltage();
                // Боковое сигнальное реле питается от линии следующего светофора
                U_side = ts->getSideVoltage();

                // Если сигнал закрыт, отключаем АЛСН-код от следующего светофора
                ts->allowTransmitALSN(!lens_state[RED_LENS]);
                return;
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::relay_control()
{

    // Сигнальное реле светофора
    bool is_SRS_ON = control_relay->getContactState(CR_ALLOW_ROUTE) &&
                     signal_relay->getContactState(SR_OPENED) &&
                     lock_relay->getContactState(LR_ROUTE_LOCKED);

    semaphore_signal_relay->setVoltage(static_cast<double>(is_SRS_ON) * U_line);


    double is_line_plus = static_cast<double>(signal_relay->getContactState(SR_OPENED));
    double is_line_minus = static_cast<double>(signal_relay->getContactState(SR_CLOSED));

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
    lens_state[RED_LENS] = semaphore_signal_relay->getContactState(SRS_N_RED);

    lens_state[YELLOW_LENS] = semaphore_signal_relay->getContactState(SRS_N_ALLOW) &&
                              (semaphore_signal_relay->getMinusContactState(SRS_MINUS_YELLOW) ||
                              (blink_contact && side_signal_relay->getContactState(SSR_YELLOW)));

    lens_state[GREEN_LENS] = semaphore_signal_relay->getContactState(SRS_N_ALLOW) &&
                             semaphore_signal_relay->getPlusContactState(SRS_PLUS_GREEN) &&
                             side_signal_relay->getContactState(SSR_GREEN);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::alsn_control()
{
    bool is_ALSN_RY_ON = semaphore_signal_relay->getContactState(SRS_N_RED);

    alsn_RY_relay->setVoltage(U_bat * static_cast<double>(is_ALSN_RY_ON));

    bool is_ALSN_Y_ON = semaphore_signal_relay->getContactState(SRS_N_ALLOW) &&
                        semaphore_signal_relay->getMinusContactState(SRS_MINUS_YELLOW);

    alsn_Y_relay->setVoltage(U_bat * static_cast<double>(is_ALSN_Y_ON));

    bool is_ALSN_G_ON = semaphore_signal_relay->getContactState(SRS_N_ALLOW) &&
                        semaphore_signal_relay->getPlusContactState(SRS_PLUS_GREEN);

    alsn_G_relay->setVoltage(U_bat * static_cast<double>(is_ALSN_G_ON));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ExitSignal::slotBlinkTimer()
{
    blink_contact = !blink_contact;
}
