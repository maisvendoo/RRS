#include    "chs2t.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void CHS2T::stepBrakesEquipment(double t, double dt)
{
    // Тормозная магистраль
    double BP_flow = 0.0;
    BP_flow += air_dist->getBPflow();
    BP_flow += brake_crane->getBPflow();
    BP_flow += epk->getBPflow();
    BP_flow += anglecock_bp_fwd->getFlowToPipe();
    BP_flow += anglecock_bp_bwd->getFlowToPipe();
    brakepipe->setFlow(BP_flow);
    brakepipe->step(t, dt);

    // Воздухораспределитель
    air_dist->setBPpressure(brakepipe->getPressure());
    air_dist->setBCpressure(electro_air_dist->getAirdistBCpressure());
    air_dist->setSRpressure(electro_air_dist->getAirdistSRpressure());
    air_dist->step(t, dt);

    // Электровоздухораспределитель
    electro_air_dist->setAirdistBCflow(air_dist->getBCflow());
    electro_air_dist->setAirdistSRflow(air_dist->getSRflow());
    electro_air_dist->setBCpressure(brake_ref_res->getPressure());
    electro_air_dist->setSRpressure(supply_reservoir->getPressure());
    electro_air_dist->step(t, dt);

    // Запасный резервуар
    supply_reservoir->setFlow(electro_air_dist->getSRflow());
    supply_reservoir->step(t, dt);

    // Тормозные рычажные передачи тележек подключены к клапанам ЗПК
    brake_mech[TROLLEY_FWD]->setBCflow(bc_switch_valve[TROLLEY_FWD]->getOutputFlow());
    brake_mech[TROLLEY_FWD]->setAngularVelocity(0, wheel_omega[0]);
    brake_mech[TROLLEY_FWD]->setAngularVelocity(1, wheel_omega[1]);
    brake_mech[TROLLEY_FWD]->setAngularVelocity(2, wheel_omega[2]);
    brake_mech[TROLLEY_FWD]->step(t, dt);

    brake_mech[TROLLEY_BWD]->setBCflow(bc_switch_valve[TROLLEY_BWD]->getOutputFlow());
    brake_mech[TROLLEY_BWD]->setAngularVelocity(0, wheel_omega[3]);
    brake_mech[TROLLEY_BWD]->setAngularVelocity(1, wheel_omega[4]);
    brake_mech[TROLLEY_BWD]->setAngularVelocity(2, wheel_omega[5]);
    brake_mech[TROLLEY_BWD]->step(t, dt);

    Q_r[0] = brake_mech[TROLLEY_FWD]->getBrakeTorque(0);
    Q_r[1] = brake_mech[TROLLEY_FWD]->getBrakeTorque(1);
    Q_r[2] = brake_mech[TROLLEY_FWD]->getBrakeTorque(2);

    Q_r[3] = brake_mech[TROLLEY_BWD]->getBrakeTorque(0);
    Q_r[4] = brake_mech[TROLLEY_BWD]->getBrakeTorque(1);
    Q_r[5] = brake_mech[TROLLEY_BWD]->getBrakeTorque(2);

    // Концевые краны тормозной магистрали
    anglecock_bp_fwd->setPipePressure(brakepipe->getPressure());
    anglecock_bp_fwd->setHoseFlow(hose_bp_fwd->getFlow());
    anglecock_bp_fwd->step(t, dt);
    anglecock_bp_bwd->setPipePressure(brakepipe->getPressure());
    anglecock_bp_bwd->setHoseFlow(hose_bp_bwd->getFlow());
    anglecock_bp_bwd->step(t, dt);

    // Рукава тормозной магистрали
    hose_bp_fwd->setPressure(anglecock_bp_fwd->getPressureToHose());
    hose_bp_fwd->setFlowCoeff(anglecock_bp_fwd->getFlowCoeff());
    hose_bp_fwd->step(t, dt);
    hose_bp_bwd->setPressure(anglecock_bp_bwd->getPressureToHose());
    hose_bp_bwd->setFlowCoeff(anglecock_bp_bwd->getFlowCoeff());
    hose_bp_bwd->step(t, dt);
}
