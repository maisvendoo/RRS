#include "stepswitch.h"

//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
StepSwitch::StepSwitch(QObject* parent) : Device(parent)
  , V(0.0)
  , poz_d(0.0)
  , poz(0)

  , fieldStep(0)

  , n(false)
  , p(false)
  , s(false)
  , prevPos(0)
  , dropPosition(false)
  , hod(false)

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
    return (poz > 0 ) && (poz <= MPOS_S);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool StepSwitch::isSeriesParallel() const
{
    return (poz > MPOS_S ) && (poz <= MPOS_SP);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool StepSwitch::isParallel() const
{
    return (poz > MPOS_SP );
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

    hod = (poz == MPOS_S  || poz == MPOS_SP || poz == MPOS_P);

    if (ctrlState.up && !s )
    {
        poz_d += V * hs_p(MPOS_P - poz_d) * dt;
        n = false;
        if ((poz == MPOS_S || poz == MPOS_SP) && prevPos != poz)
        {
            s = true;
            prevPos = poz;
        }
        if (poz != prevPos)
            prevPos = 0;
    }

    if (ctrlState.up1 && poz < MPOS_P && !n)
    {
        poz += 1;
        poz_d = poz;
        n = true;

    }
    if (ctrlState.down1 && poz > 0 && !n)
    {
        poz -= 1;
        poz_d = poz;
        n = true;
    }
    if (ctrlState.down || dropPosition)
    {
        poz_d -= V * hs_p(poz_d) * dt;
        n = false;
    }
    if (ctrlState.zero)
    {
        n = false;
        s = false;
    }

    if ((getKeyState(KEY_Z) || p ))
    {
        p = true;
        poz_d -= V * dt;
        p = !(hod || poz == 0);
    }

    fieldStep = 1 * (ctrlState.k31 && !ctrlState.k32) +
                2 * (ctrlState.k32 && !ctrlState.k31) +
                3 * (ctrlState.k33 && !ctrlState.k31) +
                4 * (ctrlState.k31 && ctrlState.k32) +
                5 * (ctrlState.k31 && ctrlState.k33);

    poz = static_cast<int>(poz_d);
}
