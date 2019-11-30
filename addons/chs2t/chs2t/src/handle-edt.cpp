#include    "handle-edt.h"
#include    "hardware-signals.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
HandleEDT::HandleEDT(QObject *parent) : Device(parent)
  , brakeKey(0)
  , releaseKey(0)
  , pos(POS_RELEASE)
  , pos_ref(pos)
  , control_signal(0.0)
{
    std::fill(K.begin(), K.end(), 0.0);

    connect(&motionTimer, &Timer::process, this, &HandleEDT::slotHandleMove);
    motionTimer.setTimeout(0.1);
    motionTimer.firstProcess(true);
    motionTimer.start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
HandleEDT::~HandleEDT()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void HandleEDT::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    switch (pos)
    {
    case POS_RELEASE:

        control_signal = 0.0;
        break;

    case POS_HOLD:

        control_signal = -1.0;
        break;

    case POS_BRAKE:

        control_signal = 1.0;
        break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void HandleEDT::ode_system(const state_vector_t &Y,
                           state_vector_t &dYdt,
                           double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
    Q_UNUSED(t)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void HandleEDT::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    for (size_t i = 1; i < K.size(); ++i)
    {
        QString coeff = QString("K%1").arg(i);
        cfg.getDouble(secName, coeff, K[i]);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void HandleEDT::stepKeysControl(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    if (getKeyState(brakeKey))
    {
        pos_ref = POS_BRAKE;
    }
    else
    {
        if (getKeyState(releaseKey))
        {
            pos_ref = POS_RELEASE;
        }
        else
        {
            if (pos == POS_BRAKE)
                pos_ref = POS_HOLD;
            else
                pos_ref = pos;
        }
    }

    motionTimer.step(t, dt);
}

void HandleEDT::stepExternalControl(double t, double dt)
{
    if (control_signals.analogSignal[EDT_BRAKE].is_active &&
        control_signals.analogSignal[EDT_RELEASE].is_active    )
    {
        if (static_cast<bool>(control_signals.analogSignal[EDT_BRAKE].cur_value))
        {
            pos_ref = POS_BRAKE;
        }

        else if (static_cast<bool>(control_signals.analogSignal[EDT_RELEASE].cur_value))
        {
            pos_ref = POS_RELEASE;
        }

        else
        {
            pos_ref = POS_HOLD;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void HandleEDT::slotHandleMove()
{
    pos += pos_ref - pos;

    pos = cut(pos, static_cast<int>(POS_RELEASE), static_cast<int>(POS_BRAKE));
}
