#include    "station-signal.h"
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
    signal_relay->setInitContactState(SR_LOCK_RELAY_CTRL, false);

    lock_relay->read_config("combine-relay");
    lock_relay->setInitContactState(LR_ROUTE_LOCKED, false);
    lock_relay->setInitContactState(LR_NO_ROUTE, true);
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

