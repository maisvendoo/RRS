#include    <line-signal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LineSignal::LineSignal(QObject *parent) : Signal(parent)
{
    way_relay->read_config("combine-relay");
    way_relay->setInitContactState(WR_WAY_BUSY, false);

    line_relay->read_config("combine-relay");
    line_relay->setInitContactState(LR_NEUTRAL_LINE_PLUS, false);
    line_relay->setInitContactState(LR_NEUTRAL_LINE_MINIS, true);
    line_relay->setInitContactState(LR_NEUTRAL_ALLOW, false);
    line_relay->setInitContactState(LR_NEUTRAL_PROHIBITING, true);

    line_relay->setInitPlusContactState(LR_PLUS_GREEN, false);
    line_relay->setInitMinusContactState(LR_MINUS_YELLOW, false);

    side_signal_relay->read_config("combine-relay");
    side_signal_relay->setInitContactState(SSR_GREEN, true);
    side_signal_relay->setInitContactState(SSR_YELLOW, false);

    connect(blink_timer, &Timer::process, this, &LineSignal::slotBlinkTimer);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LineSignal::~LineSignal()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LineSignal::step(double t, double dt)
{
    Signal::step(t, dt);

    way_relay->step(t, dt);

    line_relay->step(t, dt);

    side_signal_relay->step(t, dt);

    blink_timer->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LineSignal::preStep(state_vector_t &Y, double t)
{
    (void)Y;
    (void)t;

    check_route();

    relay_control();

    yellow_blink_control();

    lens_state_control();

    alsn_control();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LineSignal::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
    (void)Y;
    (void)dYdt;
    (void)t;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LineSignal::check_route()
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
            return;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LineSignal::relay_control()
{
    // Включение путевого реле в релейном шкафу следующего светофора
    way_relay->setVoltage(U_way);

    // Признак того, что напряжение подается в линию
    bool is_line_ON = way_relay->getContactState(WR_WAY_BUSY);

    // Линейное реле питается от линии следующего светофора
    line_relay->setVoltage(static_cast<double>(is_line_ON) * U_line);


    double is_line_PLUS = static_cast<double>(line_relay->getContactState(LR_NEUTRAL_LINE_PLUS));
    double is_line_MINUS = static_cast<double>(line_relay->getContactState(LR_NEUTRAL_LINE_MINIS));

    // Формируем напряжение, подаваемое на линейное реле предыдущего светофора
    U_line_prev = U_bat * (is_line_PLUS - is_line_MINUS);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LineSignal::yellow_blink_control()
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
void LineSignal::lens_state_control()
{
    // Запоминаем старое состояние ламп
    old_lens_state = lens_state;

    // Управляем состоянием ламп через контакты линейного реле

    // Красный сигнал при отпадании нейтрального якоря
    lens_state[RED_LENS] = line_relay->getContactState(LR_NEUTRAL_PROHIBITING);

    // Зеленый, при притянутом нейтральном якоре и положительном питании
    // линейного реле
    lens_state[GREEN_LENS] = line_relay->getContactState(LR_NEUTRAL_ALLOW) &&
                             line_relay->getPlusContactState(LR_PLUS_GREEN) &&
                             side_signal_relay->getContactState(SSR_GREEN);

    // Желтый, при притянутом нейтральном якоре и отрицательном питании
    // линейного реле
    lens_state[YELLOW_LENS] = line_relay->getContactState(LR_NEUTRAL_ALLOW) &&
                               (line_relay->getMinusContactState(LR_MINUS_YELLOW) ||
                               (blink_contact && side_signal_relay->getContactState(SSR_YELLOW)));

    // При изменение соостояния ламп обновляем его
    if (old_lens_state != lens_state)
    {
        old_lens_state = lens_state;
        emit sendDataUpdate(this->serialize());
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LineSignal::alsn_control()
{
    if (!is_asln_transmit)
    {
        alsn_reset();
        return;
    }

    bool is_ALSN_RY_ON = line_relay->getContactState(LR_NEUTRAL_PROHIBITING);

    alsn_RY_relay->setVoltage(U_bat * static_cast<double>(is_ALSN_RY_ON));

    alsn_state[ALSN_RY_LINE] = alsn_RY_relay->getContactState(ALSN_RY);

    bool is_ALSN_Y_ON = line_relay->getContactState(LR_NEUTRAL_ALLOW) &&
                        line_relay->getMinusContactState(LR_MINUS_YELLOW);

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
void LineSignal::slotBlinkTimer()
{
    blink_contact = !blink_contact;
}
