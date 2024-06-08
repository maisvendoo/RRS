#include    "vl60k.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60k::stepBrakesControl(double t, double dt)
{
    // Блокировочное устройство
    brake_lock->setFLpressure(main_reservoir->getPressure());
    brake_lock->setBPpressure(brakepipe->getPressure());
    brake_lock->setBCpressure(bc_splitter->getInputPressure());
    brake_lock->setCraneFLflow(brake_crane->getFLflow() + loco_crane->getFLflow());
    brake_lock->setCraneBPflow(brake_crane->getBPflow());
    brake_lock->setCraneBCflow(loco_crane->getBCflow());
    brake_lock->setControl(keys);
    brake_lock->step(t, dt);

    // Поездной кран машиниста
    brake_crane->setFLpressure(brake_lock->getCraneFLpressure());
    brake_crane->setBPpressure(brake_lock->getCraneBPpressure());
    brake_crane->setControl(keys);
    brake_crane->step(t, dt);

    // Кран вспомогательного тормоза
    loco_crane->setFLpressure(brake_lock->getCraneFLpressure());
    loco_crane->setBCpressure(brake_lock->getCraneBCpressure());
    loco_crane->setILpressure(impulse_line->getPressure());
    loco_crane->setControl(keys);
    loco_crane->step(t, dt);

    // Импульсная магистраль
    double il_flow = 0.0;
    il_flow += air_dist->getBCflow();
    il_flow += loco_crane->getILflow();
    if (bc_hose_to_impulse_line)
    {
        anglecock_bc_fwd->setHoseFlow(hose_bc_fwd->getFlow());
        il_flow += anglecock_bc_fwd->getFlowToPipe();

        anglecock_bc_bwd->setHoseFlow(hose_bc_bwd->getFlow());
        il_flow += anglecock_bc_bwd->getFlowToPipe();
    }
    impulse_line->setFlow(il_flow);
    impulse_line->step(t, dt);

    // Тройник подключения тележек к магистрали тормозных цилиндров
    double bc_flow = 0.0;
    bc_flow += brake_lock->getBCflow();
    if (!bc_hose_to_impulse_line)
    {
        anglecock_bc_fwd->setHoseFlow(hose_bc_fwd->getFlow());
        bc_flow += anglecock_bc_fwd->getFlowToPipe();

        anglecock_bc_bwd->setHoseFlow(hose_bc_bwd->getFlow());
        bc_flow += anglecock_bc_bwd->getFlowToPipe();
    }
    bc_splitter->setInputFlow(bc_flow);
    bc_splitter->setPipePressure1(brake_mech[TROLLEY_FWD]->getBCpressure());
    bc_splitter->setPipePressure2(brake_mech[TROLLEY_BWD]->getBCpressure());
    bc_splitter->step(t, dt);

    // Концевые краны магистрали тормозных цилиндров
    if (bc_hose_to_impulse_line)
    {
        anglecock_bc_fwd->setPipePressure(impulse_line->getPressure());
        anglecock_bc_bwd->setPipePressure(impulse_line->getPressure());
    }
    else
    {
        anglecock_bc_fwd->setPipePressure(bc_splitter->getInputPressure());
        anglecock_bc_bwd->setPipePressure(bc_splitter->getInputPressure());
    }
    //anglecock_bc_fwd->setControl(keys);
    anglecock_bc_fwd->step(t, dt);
    //anglecock_bc_bwd->setControl(keys);
    anglecock_bc_bwd->step(t, dt);

    // Рукава магистрали тормозных цилиндров
    hose_bc_fwd->setPressure(anglecock_bc_fwd->getPressureToHose());
    hose_bc_fwd->setFlowCoeff(anglecock_bc_fwd->getFlowCoeff());
    hose_bc_fwd->setCoord(railway_coord + dir * orient * (length / 2.0 - anglecock_bc_fwd->getShiftCoord()));
    hose_bc_fwd->setShiftSide(anglecock_bc_fwd->getShiftSide());
    //hose_bc_fwd->setControl(keys);
    hose_bc_fwd->step(t, dt);

    hose_bc_bwd->setPressure(anglecock_bc_bwd->getPressureToHose());
    hose_bc_bwd->setFlowCoeff(anglecock_bc_bwd->getFlowCoeff());
    hose_bc_bwd->setCoord(railway_coord + dir * orient * (length / 2.0 - anglecock_bc_bwd->getShiftCoord()));
    hose_bc_bwd->setShiftSide(anglecock_bc_bwd->getShiftSide());
    //hose_bc_bwd->setControl(keys);
    hose_bc_bwd->step(t, dt);
}
