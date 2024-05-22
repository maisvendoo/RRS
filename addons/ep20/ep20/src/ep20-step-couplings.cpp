#include    "ep20.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void EP20::stepCouplings(double t, double dt)
{
    coupling_fwd->setCoord(railway_coord + dir * orient * length / 2.0);
    coupling_fwd->setVelocity(dir * orient * velocity);

    coupling_bwd->setCoord(railway_coord - dir * orient * length / 2.0);
    coupling_bwd->setVelocity(dir * orient * velocity);

    oper_rod_fwd->setCouplingForce(coupling_fwd->getCurrentForce());
    oper_rod_fwd->setControl(keys);
    oper_rod_fwd->step(t, dt);
    coupling_fwd->setCouplingOperatingState(oper_rod_fwd->getOperatingState());
    coupling_fwd->step(t, dt);
    F_fwd += coupling_fwd->getCurrentForce();

    oper_rod_bwd->setCouplingForce(coupling_bwd->getCurrentForce());
    oper_rod_bwd->setControl(keys);
    oper_rod_bwd->step(t, dt);
    coupling_bwd->setCouplingOperatingState(oper_rod_bwd->getOperatingState());
    coupling_bwd->step(t, dt);
    F_bwd += coupling_bwd->getCurrentForce();
}
