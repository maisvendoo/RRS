#include "stepswitch.h"

//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
StepSwitch::StepSwitch(QObject* parent) : Device(parent)
  , V(0.0)
  , poz_d(0.0)
  , poz(0)

  , fieldStep(0)
  , reverseState(0)

  , up(false)
  , up1(false)
  , zero(false)
  , down1(false)
  , down(false)

  , onePositionIsChanged(false)
  , dropPositionsWithZ(false)
  , ableToGainPositions(false)

  , prevPos(0)
  , prevPos2(0)
  , dropPosition(false)
  , hod(false)
  , ctrlState(ControllerState())
  , ableToChangeOnePosition(false)
{

}

//------------------------------------------------------------------------------
// Деструктор
//------------------------------------------------------------------------------
StepSwitch::~StepSwitch()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double StepSwitch::getSchemeState() const
{
    return static_cast<double>(poz != 0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool StepSwitch::isZero() const
{
    return poz == 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool StepSwitch::isSeries() const
{
    return (poz > 0 ) && (poz <= MPOS_S) && !hod;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool StepSwitch::isSeriesParallel() const
{
    return (poz > MPOS_S ) && (poz <= MPOS_SP) &&!hod;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool StepSwitch::isParallel() const
{
    return (poz > MPOS_SP ) && !hod;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ampermeters_state_t StepSwitch::getAmpermetersState()
{
    ampermeters_state_t state;

    state.is12on = poz >= MPOS_S;
    state.is34on = poz > MPOS_SP;
    state.is56on = true;

    return state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StepSwitch::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
    Q_UNUSED(t)
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
}

//------------------------------------------------------------------------------
// Загрузка данных из конфигурационного файла
//------------------------------------------------------------------------------
void StepSwitch::load_config(CfgReader& cfg)
{
    cfg.getDouble("Device", "V", V);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StepSwitch::preStep(state_vector_t& Y, double t)
{
    Q_UNUSED(t)
    Q_UNUSED(Y)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StepSwitch::stepDiscrete(double t, double dt)
{
    Q_UNUSED(t)

    up = (ctrlState.k21 && ctrlState.k23);
    up1 = (!ctrlState.k21 && ctrlState.k23);
    zero = (ctrlState.k22 && !ctrlState.k23);
    down1 = (!ctrlState.k21 && !ctrlState.k22);
    down = (ctrlState.k21 && !ctrlState.k22);

    hod = (poz == MPOS_S  || poz == MPOS_SP || poz == MPOS_P);

    if (up && ableToGainPositions )
    {
        poz_d += V * hs_p(MPOS_P - poz_d) * dt;

        if (hod && prevPos != poz)
        {
            ableToGainPositions = false;
            prevPos = poz;
        }
    }

    changeOnePosition(up1 - down1);

    if (down || dropPosition)
    {
        poz_d -= V * hs_p(poz_d) * dt;
        fieldStep = 0;
    }
    else
    {
        fieldStep = (1 * (ctrlState.k31 && !ctrlState.k32) +
                    2 * (ctrlState.k32 && !ctrlState.k31) +
                    3 * (ctrlState.k33 && !ctrlState.k31) +
                    4 * (ctrlState.k31 && ctrlState.k32) +
                    5 * (ctrlState.k31 && ctrlState.k33)) * hod;
    }

    if (zero)
    {
        ableToChangeOnePosition = true;
        ableToGainPositions = true;
    }

    if (getKeyState(KEY_K))
    {
        prevPos2 = poz;
        dropPositionsWithZ = true;
    }

    if (dropPositionsWithZ)
    {
        poz_d -= V * dt;

        if ((poz == 0 || hod) && (poz != prevPos2))
        {
            dropPositionsWithZ = false;
        }
    }

    reverseState = (-1 * (!ctrlState.k01 && ctrlState.k02)) +
                    (1 * (ctrlState.k01 && !ctrlState.k02));

    poz = static_cast<int>(poz_d);
}

void StepSwitch::changeOnePosition(int dir)
{
    if (ableToChangeOnePosition)
    {
        poz_d += dir;
        poz_d = cut(poz_d, 0.0, static_cast<double>(MPOS_P));
        ableToChangeOnePosition = false;
    }
}
