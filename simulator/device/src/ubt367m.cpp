#include    "ubt367m.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeLock::BrakeLock(QObject *parent) : BrakeDevice(parent)
  , V0(1e-3)
  , p1(0.1)
  , state(0)
  , crane_pTM(0.0)
  , loco_pFL(0.0)
  , crane_pFL(0.0)
  , comb_crane_pos(-1)
  , handle_unlocked(true)

{
    std::fill(K.begin(), K.end(), 0.0);

    incCompCrane = new Timer(0.3);
    decCompCrane = new Timer(0.3);

    connect(incCompCrane, &Timer::process, this, &BrakeLock::combCraneInc);
    connect(decCompCrane, &Timer::process, this, &BrakeLock::combCraneDec);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeLock::~BrakeLock()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::init(double pTM, double pFL)
{
    Q_UNUSED(pFL)
    Q_UNUSED(pTM)

    setY(0, 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::setCraneTMpressure(double press)
{
    crane_pTM = press;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::setLocoFLpressure(double press)
{
    loco_pFL = press;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeLock::getState() const
{
    return state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeLock::getCraneFLpressure() const
{
    return crane_pFL;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeLock::getLocoTMpressure() const
{
    return getY(0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float BrakeLock::getCombCranePos() const
{
    return static_cast<float>(comb_crane_pos);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float BrakeLock::getMainHandlePos() const
{
    return static_cast<float>(1.0 - state);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::preStep(state_vector_t &Y, double t)
{
    handle_unlocked = static_cast<bool>(hs_n(Y[0] - p1));

    crane_pFL = loco_pFL * state;

    switch (comb_crane_pos)
    {
    case -1:

        is_emerg_brake = 0.0;
        is_comb_opened = 0.0;
        break;

    case 0:

        is_emerg_brake = 0.0;
        is_comb_opened = 1.0;
        break;

    case 1:

        is_emerg_brake = 1.0;
        is_comb_opened = 0.0;
        break;

    default:

        break;

    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::ode_system(const state_vector_t &Y,
                           state_vector_t &dYdt,
                           double t)
{
    double Q0 = K[2] * (crane_pTM * state - Y[0]) * is_comb_opened - K[1] * Y[0] * is_emerg_brake;

    dYdt[0] = Q0 / V0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::load_config(CfgReader &cfg)
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
void BrakeLock::stepKeysControl(double t, double dt)
{
    if (getKeyState(KEY_L))
    {
        if (keys[KEY_Shift_L] || keys[KEY_Shift_R])
        {
            if (!incCompCrane->isStarted())
                incCompCrane->start();
        }
        else
        {
            incCompCrane->stop();

            if (!decCompCrane->isStarted())
                decCompCrane->start();
        }
    }
    else
    {
        incCompCrane->stop();
        decCompCrane->stop();
    }

    if (getKeyState(KEY_BackSpace) && handle_unlocked)
    {
        if (keys[KEY_Shift_L] || keys[KEY_Shift_R])
        {
            setState(1);
        }
        else
        {
            setState(0);
        }
    }

    incCompCrane->step(t, dt);
    decCompCrane->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::combCraneInc()
{
    int old_pos = comb_crane_pos;

    comb_crane_pos++;

    comb_crane_pos = cut(comb_crane_pos, -1, 1);

    if (old_pos != comb_crane_pos)
        emit soundPlay("Komb_kran");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::combCraneDec()
{
    int old_pos = comb_crane_pos;

    comb_crane_pos--;

    comb_crane_pos = cut(comb_crane_pos, -1, 1);

    if (old_pos != comb_crane_pos)
        emit soundPlay("Komb_kran");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::setState(int state)
{
    int prev_state = this->state;
    this->state = state;

    if (prev_state != state)
        emit soundPlay("UBT_367_ruk");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::setCombineCranePos(int pos)
{
    comb_crane_pos = pos;
}
