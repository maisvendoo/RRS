#include "stepswitch.h"

//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
StepSwitch::StepSwitch(QObject* parent) : Device(parent)
  , V(0.0)
  , V1(0.0)
  , poz_d(0)
  , poz(0)
  , n(0)
  , p(0)
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

}

//------------------------------------------------------------------------------
// Загрузка данных из конфигурационного файла
//------------------------------------------------------------------------------
void StepSwitch::load_config(CfgReader& cfg)
{
    cfg.getDouble("Device", "V", V);
    cfg.getDouble("Device", "V1", V1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StepSwitch::preStep(state_vector_t& Y, double t)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StepSwitch::stepKeysControl(double t, double dt)
{
    hod = (poz == MPOS_S  || poz == MPOS_SP || poz == MPOS_P);

    if (ctrlState.up)
    {
        poz_d += V * hs_p(MPOS_P - poz_d) * dt;
        n = 0;
    }
    if (ctrlState.up1 && poz_d < MPOS_P && n == 0)
    {
        poz += 1;
        poz_d = poz;
        n = 1;
    }
    if (ctrlState.down1 && poz_d > 0 && n == 0)
    {
        poz -= 1;
        poz_d = poz;
        n = 1;
    }
    if (ctrlState.down)
    {
        poz_d -= V * hs_p(poz_d) * dt;
        n = 0;
    }
    if (ctrlState.zero)
    {
        n = 0;
    }

    if ((getKeyState(KEY_Z) || p == 1))
    {
//        if (poz > MPOS_SP)
//        {
//            poz_d -= V1 * hs_p(poz - MPOS_SP) * dt;
//            p = hs_p(poz_d - MPOS_SP);
//        }
//        else if (poz > MPOS_S)
//        {
//            poz_d -= V1 * hs_p(poz - MPOS_S) * dt;
//            p = hs_p(poz_d - MPOS_S);
//        }
//        else if (poz > 0)
//        {
//            poz_d -= V1 * hs_p(poz) * dt;
//            p = hs_p(poz_d);
//         }

        p = 1;
        poz_d -= V1 * dt;
        if (hod || poz == 0)
            p = 0;
    }

    poz = static_cast<int>(poz_d);



    hod = (poz == MPOS_S  || poz == MPOS_SP || poz == MPOS_P);


}
