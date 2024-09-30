#include    <line-signal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LineSignal::LineSignal(QObject *parent) : Signal(parent)
{
    way_relay->read_config("combine-relay");
    way_relay->setInitContactState(WR_NEUTRAL_WAY_BUSY, false);

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
    lens_state[YELLOW_LENS] = (line_relay->getContactState(LR_NEUTRAL_ALLOW) &&
                               line_relay->getMinusContactState(LR_MINUS_YELLOW)) ||
                              (blink_contact && side_signal_relay->getContactState(SSR_YELLOW));

    // При изменение соостояния ламп обновляем его
    if (old_lens_state != lens_state)
    {
        old_lens_state = lens_state;
        emit sendDataUpdate(this->serialize());
    }

    // Признак  замыкания цепи путевой батареи
    double is_closed = static_cast<double>(is_busy);

    // определяющий включение путевого реле
    way_relay->setVoltage(U_bat * (1.0 - is_closed));

    // Линейное реле питается от линни следующего светофора
    line_relay->setVoltage(U_line);

    // Формируем напряжение, подаваемое на линейное реле предыдущего светофора
    double U_line_prev_old = U_line_prev;

    // Признак того, что напряжение подается в линию
    double is_line_relay_ON = static_cast<double>(way_relay->getContactState(WR_NEUTRAL_WAY_BUSY));

    double is_line_power_PLUS = static_cast<double>(line_relay->getContactState(LR_NEUTRAL_LINE_PLUS));

    double is_line_power_MINUS = static_cast<double>(line_relay->getContactState(LR_NEUTRAL_LINE_MINIS));

    U_line_prev = is_line_relay_ON * U_bat * (is_line_power_PLUS - is_line_power_MINUS);

    // Если напряжение питание изменилось
    if (qAbs(U_line_prev - U_line_prev_old) >= 1.0)
    {
        // Устанавливаем новое значение на линии предыдущего светофора
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

    // Задаем состояние сигнальных линий АЛСН
    alsn_state[ALSN_RY_LINE] = lens_state[RED_LENS];

    alsn_state[ALSN_G_LINE] = lens_state[GREEN_LENS];

    alsn_state[ALSN_Y_LINE] = (line_relay->getContactState(LR_NEUTRAL_ALLOW) &&
                               line_relay->getMinusContactState(LR_MINUS_YELLOW)) ||
                              (side_signal_relay->getContactState(SSR_YELLOW));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LineSignal::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LineSignal::slotBlinkTimer()
{
    blink_contact = !blink_contact;
}
