#include    "brake-lock.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeLock::BrakeLock(QObject *parent) : BrakeDevice(parent)
  , state(false)
  , is_handle_locked(false)
  , p_lock(0.1)
  , comb_crane_pos(0)
  , pFL(0.0)
  , pBP(0.0)
  , pBC(0.0)
  , QFL(0.0)
  , QBP(0.0)
  , QBC(0.0)
  , V0(1e-3)
  , K_emergency(1e-3)
{
    incCombCrane = new Timer(0.3);
    decCombCrane = new Timer(0.3);

    connect(incCombCrane, &Timer::process, this, &BrakeLock::combCraneInc);
    connect(decCombCrane, &Timer::process, this, &BrakeLock::combCraneDec);
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
void BrakeLock::init(double pBP, double pFL)
{
    Q_UNUSED(pFL)
    Q_UNUSED(pBP)

    setY(0, 0.0);
    setY(1, 0.0);
    setY(2, 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool BrakeLock::setState(bool state)
{
    if (is_handle_locked)
        return false;

    bool prev_state = this->state;
    this->state = state;

    if (prev_state != state)
        emit soundPlay("UBT_367_ruk");

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool BrakeLock::isUnlocked() const
{
    return state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool BrakeLock::isMainHandleLocked() const
{
    return is_handle_locked;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeLock::getMainHandlePosition() const
{
    return static_cast<double>(!state);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::setCombineCranePosition(int pos)
{
    comb_crane_pos = pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeLock::getCombCranePosition() const
{
    return static_cast<double>(comb_crane_pos);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::setFLpressure(double value)
{
    pFL = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeLock::getCraneFLpressure() const
{
    return getY(0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::setCraneFLflow(double value)
{
    QFL = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeLock::getFLflow() const
{
    if (state)
        return QFL;
    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::setBPpressure(double value)
{
    pBP = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeLock::getCraneBPpressure() const
{
    return getY(1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::setCraneBPflow(double value)
{
    QBP = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeLock::getBPflow() const
{
    if (comb_crane_pos == 1)
        return -K_emergency * pBP;
    if (state && (comb_crane_pos == 0))
        return QBP;
    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::setBCpressure(double value)
{
    pBC = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeLock::getCraneBCpressure() const
{
    return getY(2);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::setCraneBCflow(double value)
{
    QBC = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeLock::getBCflow() const
{
    if (state)
        return QBC;
    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    is_handle_locked = static_cast<bool>(hs_p(pBP - p_lock));

    if (state)
    {
        Y[0] = pFL;

        if (comb_crane_pos == 0)
            Y[1] = pBP;

        Y[2] = pBC;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::ode_system(const state_vector_t &Y,
                           state_vector_t &dYdt,
                           double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    if (state)
    {
        dYdt[0] = 0.0;

        if (comb_crane_pos == 0)
            dYdt[1] = 0.0;
        else
            dYdt[1] = QBP / V0;

        dYdt[2] = 0.0;
    }
    else
    {
        dYdt[0] = QFL / V0;
        dYdt[1] = QBP / V0;
        dYdt[2] = QBC / V0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    double tmp = 0.0;
    cfg.getDouble(secName, "V0", tmp);
    if (tmp > 1e-3)
        V0 = tmp;

    cfg.getDouble(secName, "p_lock", p_lock);

    cfg.getDouble(secName, "K_emergency", K_emergency);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeLock::stepKeysControl(double t, double dt)
{
    // Управление ручкой блокировочного устройства
    if (getKeyState(KEY_BackSpace))
    {
        setState(isShift());
    }

    // Управление комбинированным краном
    if (getKeyState(KEY_L))
    {
        if (isShift())
        {
            decCombCrane->stop();

            if (!incCombCrane->isStarted())
                incCombCrane->start();
        }
        else
        {
            incCombCrane->stop();

            if (!decCombCrane->isStarted())
                decCombCrane->start();
        }
    }
    else
    {
        incCombCrane->stop();
        decCombCrane->stop();
    }

    incCombCrane->step(t, dt);
    decCombCrane->step(t, dt);
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
