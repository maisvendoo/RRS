#include    "vl60pk.h"

#include "brake-crane.h"
#include "brake-mech.h"
#include "electro-airdistributor.h"
#include "loco-crane.h"
#include "pneumo-brake-lock.h"
#include "pneumo-anglecock.h"
#include "pneumo-hose.h"
#include "pneumo-relay.h"
#include "pneumo-switching-valve.h"
#include "reservoir.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60pk::stepBrakesControl(const double& t, const double& dt)
{
    for (size_t cab_idx : {CAB1, CAB2})
    {
        // Блокировочное устройство
        brake_lock[cab_idx]->setFLpressure(main_reservoir->getPressure());
        brake_lock[cab_idx]->setBPpressure(brakepipe->getPressure());
        brake_lock[cab_idx]->setBCpressure(bc_switch_valve->getPressure1());
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
        loco_crane[cab_idx]->setILpressure(0.0);
        loco_crane[cab_idx]->step(t, dt);
    }

    // Переключательный клапан ЗПК
    // Первый вход клапана моделирует магистраль тормозных цилиндров
    // Второй вход клапана подключен к воздухораспределителю
    // Выход клапана подключен к ТЦ задней тележки
    double bc_flow1 = 0.0;
    bc_flow1 += brake_lock[CAB1]->getBCflow();
    bc_flow1 += brake_lock[CAB2]->getBCflow();

    anglecock_bc_fwd->setHoseFlow(hose_bc_fwd->getFlow());
    bc_flow1 += anglecock_bc_fwd->getFlowToPipe();

    anglecock_bc_bwd->setHoseFlow(hose_bc_bwd->getFlow());
    bc_flow1 += anglecock_bc_bwd->getFlowToPipe();

    bc_switch_valve->setInputFlow1(bc_flow1);
    bc_switch_valve->setInputFlow2(electro_air_dist->getBCflow());
    bc_switch_valve->setOutputPressure(brake_mech[TROLLEY_BWD]->getBCpressure());
    bc_switch_valve->step(t, dt);

    // Повторительное реле давления №304
    // Управляет давлением в ТЦ передней тележки по давлению в ТЦ задней тележки
    bc_pressure_relay->setFLpressure(main_reservoir->getPressure());
    bc_pressure_relay->setControlPressure(brake_mech[TROLLEY_BWD]->getBCpressure());
    bc_pressure_relay->setPipePressure(brake_mech[TROLLEY_FWD]->getBCpressure());
    bc_pressure_relay->step(t, dt);

    // Концевые краны магистрали тормозных цилиндров
    anglecock_bc_fwd->setPipePressure(bc_switch_valve->getPressure1());
    anglecock_bc_fwd->step(t, dt);

    anglecock_bc_bwd->setPipePressure(bc_switch_valve->getPressure1());
    anglecock_bc_bwd->step(t, dt);

    // Рукава магистрали тормозных цилиндров
    hose_bc_fwd->setPressure(anglecock_bc_fwd->getPressureToHose());
    hose_bc_fwd->setFlowCoeff(anglecock_bc_fwd->getFlowCoeff());
    hose_bc_fwd->setCoord(train_coord + dir * orient * (length / 2.0 - anglecock_bc_fwd->getShiftCoord()));
    hose_bc_fwd->setShiftSide(anglecock_bc_fwd->getShiftSide());
    hose_bc_fwd->step(t, dt);

    hose_bc_bwd->setPressure(anglecock_bc_bwd->getPressureToHose());
    hose_bc_bwd->setFlowCoeff(anglecock_bc_bwd->getFlowCoeff());
    hose_bc_bwd->setCoord(train_coord - dir * orient * (length / 2.0 - anglecock_bc_bwd->getShiftCoord()));
    hose_bc_bwd->setShiftSide(anglecock_bc_bwd->getShiftSide());
    hose_bc_bwd->step(t, dt);
}
