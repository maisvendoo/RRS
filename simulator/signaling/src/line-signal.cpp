#include    "line-signal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LineSignal::LineSignal(QObject *parent) : Signal(parent)
  , busy_relay(new LogicRelay(3))
  , yellow_relay(new LogicRelay(2))
{
    busy_relay->setInitContactState(0, true);
    busy_relay->setInitContactState(1, true);
    busy_relay->setInitContactState(2, false);
    connect(busy_relay, &LogicRelay::changeState,
            this, &LineSignal::slotChangeState);

    yellow_relay->setInitContactState(0, true);
    yellow_relay->setInitContactState(1, false);
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
void LineSignal::close(bool is_closed)
{
    busy_relay->setState(!is_closed);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LineSignal::step(double t, double dt)
{
    busy_relay->step(t, dt);
    yellow_relay->step(t, dt);

    Signal::step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LineSignal::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    // Переключение на желтый при закрытии следующего сигнала
    yellow_relay->setState(prev_signal_closed);

    // Определяем состояние линз

    // Красный
    lens_state[LINE_RED] = busy_relay->getContactState(0);

    // Зеленый
    lens_state[LINE_GREEN] = busy_relay->getContactState(2) &&
            yellow_relay->getContactState(0);

    // Желтый
    lens_state[LINE_YELLOW] = busy_relay->getContactState(2) &&
            yellow_relay->getContactState(1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LineSignal::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LineSignal::slotChangeState()
{
    emit sendClosedState(busy_relay->getContactState(1));
}
