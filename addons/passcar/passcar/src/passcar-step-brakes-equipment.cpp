#include    "passcar.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void PassCar::stepBrakesEquipment(double t, double dt)
{
    // Тормозная магистраль
    double BP_flow = 0.0;
    BP_flow += air_dist->getBPflow();

    anglecock_bp_fwd->setHoseFlow(hose_bp_fwd->getFlow());
    BP_flow += anglecock_bp_fwd->getFlowToPipe();

    anglecock_bp_bwd->setHoseFlow(hose_bp_bwd->getFlow());
    BP_flow += anglecock_bp_bwd->getFlowToPipe();

    brakepipe->setFlow(BP_flow);
    brakepipe->step(t, dt);

    // Воздухораспределитель
    air_dist->setBPpressure(brakepipe->getPressure());
    if (electro_air_dist != nullptr)
    {
        air_dist->setBCpressure(electro_air_dist->getAirdistBCpressure());
        air_dist->setSRpressure(electro_air_dist->getAirdistSRpressure());
    }
    else
    {
        air_dist->setBCpressure(brake_mech->getBCpressure());
        air_dist->setSRpressure(supply_reservoir->getPressure());
    }
    air_dist->step(t, dt);

    // Электровоздухораспределитель
    if (electro_air_dist != nullptr)
    {
        electro_air_dist->setAirdistBCflow(air_dist->getBCflow());
        electro_air_dist->setAirdistSRflow(air_dist->getSRflow());
        electro_air_dist->setBCpressure(brake_mech->getBCpressure());
        electro_air_dist->setSRpressure(supply_reservoir->getPressure());
        electro_air_dist->step(t, dt);
    }

    // Запасный резервуар
    if (electro_air_dist != nullptr)
        supply_reservoir->setFlow(electro_air_dist->getSRflow());
    else
        supply_reservoir->setFlow(air_dist->getSRflow());
    supply_reservoir->step(t, dt);

    // Тормозная рычажная передача
    if (electro_air_dist != nullptr)
        brake_mech->setBCflow(electro_air_dist->getBCflow());
    else
        brake_mech->setBCflow(air_dist->getBCflow());
    brake_mech->step(t, dt);
    for (size_t i = 0; i < num_axis; ++i)
    {
        brake_mech->setAngularVelocity(i, wheel_omega[i]);
        Q_r[i + 1] = brake_mech->getBrakeTorque(i);
    }

    // Концевые краны тормозной магистрали
    anglecock_bp_fwd->setPipePressure(brakepipe->getPressure());
    anglecock_bp_fwd->setControl(keys);
    anglecock_bp_fwd->step(t, dt);
    anglecock_bp_bwd->setPipePressure(brakepipe->getPressure());
    anglecock_bp_bwd->setControl(keys);
    anglecock_bp_bwd->step(t, dt);

    // Рукава тормозной магистрали
    hose_bp_fwd->setPressure(anglecock_bp_fwd->getPressureToHose());
    hose_bp_fwd->setFlowCoeff(anglecock_bp_fwd->getFlowCoeff());
    hose_bp_fwd->setCoord(railway_coord + dir * orient * (length / 2.0 - anglecock_bp_fwd->getShiftCoord()));
    hose_bp_fwd->setShiftSide(anglecock_bp_fwd->getShiftSide());
    //hose_bp_fwd->setControl(keys);
    hose_bp_fwd->step(t, dt);
    hose_bp_bwd->setPressure(anglecock_bp_bwd->getPressureToHose());
    hose_bp_bwd->setFlowCoeff(anglecock_bp_bwd->getFlowCoeff());
    hose_bp_bwd->setCoord(railway_coord - dir * orient * (length / 2.0 - anglecock_bp_bwd->getShiftCoord()));
    hose_bp_bwd->setShiftSide(anglecock_bp_bwd->getShiftSide());
    //hose_bp_bwd->setControl(keys);
    hose_bp_bwd->step(t, dt);
}
