#include    "vl60k.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60k::preStepCouplings(double t)
{
    (void) t;

    // Координата и скорость переднего сцепного устройства (по оси сцепления)
    coupling_fwd->setCoord(train_coord + dir * orient * length / 2.0);
    coupling_fwd->setVelocity(dir * orient * velocity);

    // Координата и скорость заднего сцепного устройства (по оси сцепления)
    coupling_bwd->setCoord(train_coord - dir * orient * length / 2.0);
    coupling_bwd->setVelocity(dir * orient * velocity);
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60k::stepCouplings(double t, double dt)
{
    // Управление передним сцепным устройством
    oper_rod_fwd->setCouplingForce(coupling_fwd->getCurrentForce());
    oper_rod_fwd->setControl(keys);
    oper_rod_fwd->step(t, dt);
    coupling_fwd->setCouplingOperatingState(oper_rod_fwd->getOperatingState());
    coupling_fwd->step(t, dt);
    // Добавляем усилие от переднего сцепного устройства на данном шаге
    F_fwd += coupling_fwd->getCurrentForce();

    // Управление задним сцепным устройством
    oper_rod_bwd->setCouplingForce(coupling_bwd->getCurrentForce());
    oper_rod_bwd->setControl(keys);
    oper_rod_bwd->step(t, dt);
    coupling_bwd->setCouplingOperatingState(oper_rod_bwd->getOperatingState());
    coupling_bwd->step(t, dt);
    // Добавляем усилие от заднего сцепного устройства на данном шаге
    F_bwd += coupling_bwd->getCurrentForce();
}
