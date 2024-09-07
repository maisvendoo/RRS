#include    "freightcar.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void FreightCar::stepBrakesEquipment(double t, double dt)
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

    // Подключение воздухораспределителя к ТЦ или к авторежиму
    double ad_bc_flow = 0.0;
    double ad_sr_flow = 0.0;
    double bc_ad_pressure = brake_mech->getBCpressure();
    if (automode != nullptr)
    {
        bc_ad_pressure = automode->getAirDistBCpressure();
    }

    // Воздухораспределитель
    air_dist->setBPpressure(brakepipe->getPressure());
    if (electro_air_dist != nullptr)
    {
        air_dist->setBCpressure(electro_air_dist->getAirdistBCpressure());
        air_dist->setSRpressure(electro_air_dist->getAirdistSRpressure());

        ad_bc_flow = electro_air_dist->getBCflow();
        ad_sr_flow = electro_air_dist->getSRflow();
    }
    else
    {
        air_dist->setBCpressure(bc_ad_pressure);
        air_dist->setSRpressure(supply_reservoir->getPressure());

        ad_bc_flow = air_dist->getBCflow();
        ad_sr_flow = air_dist->getSRflow();
    }
    air_dist->step(t, dt);

    // Электровоздухораспределитель
    if (electro_air_dist != nullptr)
    {
        electro_air_dist->setAirdistBCflow(air_dist->getBCflow());
        electro_air_dist->setAirdistSRflow(air_dist->getSRflow());
        electro_air_dist->setBCpressure(bc_ad_pressure);
        electro_air_dist->setSRpressure(supply_reservoir->getPressure());
        electro_air_dist->step(t, dt);
    }

    // Запасный резервуар
    supply_reservoir->setFlow(ad_sr_flow);
    supply_reservoir->step(t, dt);

    // Авторежим
    if (automode != nullptr)
    {
        automode->setAirDistBCflow(ad_bc_flow);
        automode->setBCpressure(brake_mech->getBCpressure());
        automode->setPayloadCoeff(payload_coeff);
        automode->step(t, dt);

        ad_bc_flow = automode->getBCflow();
    }

    // Тормозная рычажная передача
    brake_mech->setBCflow(ad_bc_flow);
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
    hose_bp_fwd->setCoord(train_coord + dir * orient * (length / 2.0 - anglecock_bp_fwd->getShiftCoord()));
    hose_bp_fwd->setShiftSide(anglecock_bp_fwd->getShiftSide());
    hose_bp_fwd->setControl(keys);
    hose_bp_fwd->step(t, dt);
    hose_bp_bwd->setPressure(anglecock_bp_bwd->getPressureToHose());
    hose_bp_bwd->setFlowCoeff(anglecock_bp_bwd->getFlowCoeff());
    hose_bp_bwd->setCoord(train_coord - dir * orient * (length / 2.0 - anglecock_bp_bwd->getShiftCoord()));
    hose_bp_bwd->setShiftSide(anglecock_bp_bwd->getShiftSide());
    hose_bp_bwd->setControl(keys);
    hose_bp_bwd->step(t, dt);
}
