#include    "vl60k.h"

#include "airdistributor.h"
#include "brake-crane.h"
#include "brake-mech.h"
#include "loco-crane.h"
#include "pneumo-brake-lock.h"
#include "pneumo-anglecock.h"
#include "pneumo-hose.h"
#include "pneumo-splitter.h"
#include "reservoir.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60k::stepBrakesControl(const double& t, const double& dt)
{
    for (size_t cab_idx : {CAB1, CAB2})
    {
        // Блокировочное устройство
        brake_lock[cab_idx]->setFLpressure(main_reservoir->getPressure());
        brake_lock[cab_idx]->setBPpressure(brakepipe->getPressure());
        brake_lock[cab_idx]->setBCpressure(bc_splitter->getInputPressure());
        brake_lock[cab_idx]->setCraneFLflow(brake_crane[cab_idx]->getFLflow() + loco_crane[cab_idx]->getFLflow());
        brake_lock[cab_idx]->setCraneBPflow(brake_crane[cab_idx]->getBPflow());
        brake_lock[cab_idx]->setCraneBCflow(loco_crane[cab_idx]->getBCflow());
        brake_lock[cab_idx]->step(t, dt);

        // Поездной кран машиниста
        brake_crane[cab_idx]->setFLpressure(brake_lock[cab_idx]->getCraneFLpressure());
        brake_crane[cab_idx]->setBPpressure(brake_lock[cab_idx]->getCraneBPpressure());
        brake_crane[cab_idx]->step(t, dt);

        // Кран вспомогательного тормоза
        loco_crane[cab_idx]->setFLpressure(brake_lock[cab_idx]->getCraneFLpressure());
        loco_crane[cab_idx]->setBCpressure(brake_lock[cab_idx]->getCraneBCpressure());
        loco_crane[cab_idx]->setILpressure(impulse_line->getPressure());
        loco_crane[cab_idx]->step(t, dt);
    }

    // Импульсная магистраль
    double il_flow = 0.0;
    il_flow += air_dist->getBCflow();
    il_flow += loco_crane[CAB1]->getILflow();
    il_flow += loco_crane[CAB2]->getILflow();
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
    bc_flow += brake_lock[CAB1]->getBCflow();
    bc_flow += brake_lock[CAB2]->getBCflow();
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
    anglecock_bc_fwd->step(t, dt);
    anglecock_bc_bwd->step(t, dt);

    // Рукава магистрали тормозных цилиндров
    hose_bc_fwd->setPressure(anglecock_bc_fwd->getPressureToHose());
    hose_bc_fwd->setFlowCoeff(anglecock_bc_fwd->getFlowCoeff());
    hose_bc_fwd->setCoord(train_coord + dir * (length / 2.0 - anglecock_bc_fwd->getShiftCoord()));
    hose_bc_fwd->setShiftSide(anglecock_bc_fwd->getShiftSide());
    hose_bc_fwd->step(t, dt);

    hose_bc_bwd->setPressure(anglecock_bc_bwd->getPressureToHose());
    hose_bc_bwd->setFlowCoeff(anglecock_bc_bwd->getFlowCoeff());
    hose_bc_bwd->setCoord(train_coord - dir * (length / 2.0 - anglecock_bc_bwd->getShiftCoord()));
    hose_bc_bwd->setShiftSide(anglecock_bc_bwd->getShiftSide());
    hose_bc_bwd->step(t, dt);
}
