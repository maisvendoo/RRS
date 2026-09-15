#include    "ep1m.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void EP1m::stepBrakesControl(const double& t, const double& dt)
{
    for (size_t cab_idx : {CAB1, CAB2})
    {
        // Блокировочное устройство
        brake_lock[cab_idx]->setFLpressure(main_reservoir->getPressure());
        brake_lock[cab_idx]->setBPpressure(brakepipe->getPressure());
        brake_lock[cab_idx]->setBCpressure(kp5->getPressure1());
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

        // ЭПК
        epk[cab_idx]->setFLpressure(main_reservoir->getPressure());
        epk[cab_idx]->setBPpressure(brakepipe->getPressure());
        epk[cab_idx]->setPowered(klub_BEL->getEPKstate());
        epk[cab_idx]->step(t, dt);
    }

    // Повторительное пневмореле для давления от воздухораспределителя РД4
    // Управляющая камера моделирует импульсный резервуар (ложный ТЦ)
    rd4->setFLpressure(main_reservoir->getPressure());
    rd4->setControlFlow(electro_air_dist->getBCflow());
    rd4->setPipePressure(kp1->getPressure2());
    rd4->step(t, dt);

    // Панель редукторов
    pneumo_red_panel->setFLpressure(main_reservoir->getPressure());
    pneumo_red_panel->setPressure1(Y4->getPressureToDevice());
    pneumo_red_panel->setPressure5(Y5->getPressureToDevice());
    pneumo_red_panel->step(t, dt);

    // Вентиль отпуска тормозов У1
    Y3->setDeviceFlow(kp1->getOutputFlow());
    Y3->setPipePressure(kp2->getPressure1());
    Y3->step(t, dt);

    // Вентиль замещения ЭДТ У4
    Y4->setDeviceFlow(pneumo_red_panel->getOutputFlow1());
    Y4->setPipePressure(kp2->getPressure2());
    Y4->step(t, dt);

    // Вентиль усилителя торможения У5
    Y5->setVoltage(Ucc * static_cast<double>(msud->getOutputData().is_not_brake_boost));
    Y5->setDeviceFlow(pneumo_red_panel->getOutputFlow5());
    Y5->setPipePressure(kp1->getPressure1());
    Y5->step(t, dt);

    // Переключательный клапан КП1
    // Входы от усилителя торможения У5 и повторителя давления от воздухораспределителя
    kp1->setInputFlow1(Y5->getFlowToPipe());
    kp1->setInputFlow2(rd4->getPipeFlow());
    kp1->setOutputPressure(Y3->getPressureToDevice());
    kp1->step(t, dt);

    // Переключательный клапан КП2
    // Входы от отпускного вентиля У3 и от вентиля замещения У4
    kp2->setInputFlow1(Y3->getFlowToPipe());
    kp2->setInputFlow2(Y4->getFlowToPipe());
    kp2->setOutputPressure(kp5->getPressure2());
    kp2->step(t, dt);

    // Переключательный клапан КП5
    // Входы от крана локомотивного тормоза через УБТ и от клапана КП2
    // Выход клапана подключен через тройники к повторителям давления тележек
    kp5->setInputFlow1(brake_lock[CAB1]->getBCflow() + brake_lock[CAB2]->getBCflow());
    kp5->setInputFlow2(kp2->getOutputFlow());
    kp5->setOutputPressure(bc_splitter[0]->getInputPressure());
    kp5->step(t, dt);

    // Тройники
    bc_splitter[0]->setInputFlow(kp5->getOutputFlow());
    bc_splitter[0]->setPipePressure1(bc_pressure_relay[TROLLEY_MID]->getControlPressure());
    bc_splitter[0]->setPipePressure2(bc_splitter[1]->getInputPressure());
    bc_splitter[0]->step(t, dt);

    bc_splitter[1]->setInputFlow(bc_splitter[0]->getPipeFlow2());
    bc_splitter[1]->setPipePressure1(bc_pressure_relay[TROLLEY_FWD]->getControlPressure());
    bc_splitter[1]->setPipePressure2(bc_pressure_relay[TROLLEY_BWD]->getControlPressure());
    bc_splitter[1]->step(t, dt);

    // Повторительное реле давления №304 передней тележки
    bc_pressure_relay[TROLLEY_FWD]->setFLpressure(main_reservoir->getPressure());
    bc_pressure_relay[TROLLEY_FWD]->setControlFlow(bc_splitter[1]->getPipeFlow1());
    bc_pressure_relay[TROLLEY_FWD]->setPipePressure(brake_mech[TROLLEY_FWD]->getBCpressure());
    bc_pressure_relay[TROLLEY_FWD]->step(t, dt);

    // Повторительное реле давления №304 средней тележки
    bc_pressure_relay[TROLLEY_MID]->setFLpressure(main_reservoir->getPressure());
    bc_pressure_relay[TROLLEY_MID]->setControlFlow(bc_splitter[0]->getPipeFlow1());
    bc_pressure_relay[TROLLEY_MID]->setPipePressure(brake_mech[TROLLEY_MID]->getBCpressure());
    bc_pressure_relay[TROLLEY_MID]->step(t, dt);

    // Повторительное реле давления №304 задней тележки
    bc_pressure_relay[TROLLEY_BWD]->setFLpressure(main_reservoir->getPressure());
    bc_pressure_relay[TROLLEY_BWD]->setControlFlow(bc_splitter[1]->getPipeFlow2());
    bc_pressure_relay[TROLLEY_BWD]->setPipePressure(brake_mech[TROLLEY_BWD]->getBCpressure());
    bc_pressure_relay[TROLLEY_BWD]->step(t, dt);
}
