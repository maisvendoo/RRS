#include    "train-signal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainSignal::TrainSignal(QObject* parent) : Signal(parent)
{
    std::fill(alsn_state.begin(), alsn_state.end(), false);

    alsn_RY_relay->read_config("combine-relay");
    alsn_RY_relay->setInitContactState(ALSN_RY, false);

    alsn_Y_relay->read_config("combine-relay");
    alsn_Y_relay->setInitContactState(ALSN_Y, false);

    alsn_G_relay->read_config("combine-relay");
    alsn_G_relay->setInitContactState(ALSN_G, false);

    connect(alsn_allow_timer, &Timer::process, this, &TrainSignal::slotAllowTransmit);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainSignal::~TrainSignal()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainSignal::step(double t, double dt)
{
    Signal::step(t, dt);

    alsn_RY_relay->step(t, dt);
    alsn_Y_relay->step(t, dt);
    alsn_G_relay->step(t, dt);

    if (is_alsn_allow)
    {
        // Если АЛСН не запрещена, с задержкой по 10-секундному таймеру включаем трансмиттер
        if (!is_asln_transmit)
        {
            if (!alsn_allow_timer->isStarted())
                alsn_allow_timer->start();

            alsn_allow_timer->step(t, dt);
        }
    }
    else
    {
        is_asln_transmit = false;

        // Сбрасываем запрет АЛСН
        is_alsn_allow = true;
    }

    if (!is_asln_transmit)
    {
        std::fill(alsn_state.begin(), alsn_state.end(), false);
        return;
    }

    alsn_state[ALSN_RY_LINE] = alsn_RY_relay->getContactState(ALSN_RY);
    alsn_state[ALSN_Y_LINE] = alsn_Y_relay->getContactState(ALSN_Y);
    alsn_state[ALSN_G_LINE] = alsn_G_relay->getContactState(ALSN_G);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainSignal::slotAllowTransmit()
{
    alsn_allow_timer->stop();
    is_asln_transmit = true;
}
