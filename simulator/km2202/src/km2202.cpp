#include    "km2202.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ControllerKM2202::ControllerKM2202(QObject *parent) : TractionController(parent)
  , position(0)
  , reversor_dir(1)
  , trac_timeout(0.3)
  , revers_timeout(0.3)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ControllerKM2202::~ControllerKM2202()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::ode_system(const state_vector_t &Y,
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
void ControllerKM2202::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)

    QString secName = "Device";

    int tmp = 0;
    cfg.getInt(secName, "TracTimeout", tmp);

    trac_timeout = static_cast<double>(tmp) / 1000.0;

    incTracTimer = new Timer(trac_timeout);
    decTracTimer = new Timer(trac_timeout);

    //connect(incTracTimer, &Timer::process, this, &ControllerKM2202::inc_trac_position);
    //connect(decTracTimer, &Timer::process, this, &ControllerKM2202::dec_trac_position);
    connect(incTracTimer, SIGNAL(process()), this, SLOT(inc_trac_position()), Qt::DirectConnection);
    connect(decTracTimer, SIGNAL(process()), this, SLOT(dec_trac_position()), Qt::DirectConnection);

    cfg.getInt(secName, "ReversTimeout", tmp);
    revers_timeout = static_cast<double>(tmp) / 1000.0;

    incReversTimer = new Timer(revers_timeout);
    decReversTimer = new Timer(revers_timeout);

    //connect(incReversTimer, &Timer::process, this, &ControllerKM2202::inc_reversor_dir);
    //connect(decReversTimer, &Timer::process, this, &ControllerKM2202::dec_reversor_dir);
    connect(incReversTimer, SIGNAL(process()), this, SLOT(inc_reversor_dir()), Qt::DirectConnection);
    connect(decReversTimer, SIGNAL(process()), this, SLOT(dec_reversor_dir()), Qt::DirectConnection);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::stepKeysControl(double t, double dt)
{
    stepFeedback();

    if (getKeyState(KEY_A))
    {
        if (!incTracTimer->isStarted())
            incTracTimer->start();
    }
    else
    {
        incTracTimer->stop();
    }

    if (getKeyState(KEY_D))
    {
        if (getKeyState(KEY_Shift_L) || getKeyState(KEY_Shift_R))
            position = 0;
         else
        {
            if (!decTracTimer->isStarted())
                decTracTimer->start();
        }
    }
    else
    {
        decTracTimer->stop();
    }

    if (getKeyState(KEY_W))
    {
        if (!incReversTimer->isStarted())
            incReversTimer->start();
    }
    else
    {
        incReversTimer->stop();
    }

    if (getKeyState(KEY_S))
    {
        if (!decReversTimer->isStarted())
            decReversTimer->start();
    }
    else
    {
        decReversTimer->stop();
    }

    incTracTimer->step(t, dt);
    decTracTimer->step(t, dt);

    incReversTimer->step(t, dt);
    decReversTimer->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::inc_trac_position()
{
    if (reversor_dir == 0)
        return;

    position++;
    position = cut(position, static_cast<int>(MIN_POSITION), static_cast<int>(MAX_POSITION));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::dec_trac_position()
{
    if (reversor_dir == 0)
        return;

    position--;
    position = cut(position, static_cast<int>(MIN_POSITION), static_cast<int>(MAX_POSITION));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::inc_reversor_dir()
{
    if (position != 0)
        return;

    reversor_dir++;
    reversor_dir = cut(reversor_dir, -1, 1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::dec_reversor_dir()
{
    if (position != 0)
        return;

    reversor_dir--;
    reversor_dir = cut(reversor_dir, -1, 1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::stepFeedback()
{

}

GET_TRACTION_CONTROLLER(ControllerKM2202)
