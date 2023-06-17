#include    "ep20.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void EP20::stepBrakesControl(double t, double dt)
{
    // Поездной кран машиниста
    brake_crane->setFLpressure(main_reservoir->getPressure());
    brake_crane->setBPpressure(brakepipe->getPressure());
    brake_crane->setControl(keys);
    brake_crane->step(t, dt);

    // Кран вспомогательного тормоза
    loco_crane->setFLpressure(main_reservoir->getPressure());
    loco_crane->setBCpressure(bc_switch_valve->getPressure1());
    loco_crane->setILpressure(0.0);
    loco_crane->setControl(keys);
    loco_crane->step(t, dt);

    // Переключательный клапан ЗПК
    // Первый вход клапана моделирует магистраль тормозных цилиндров
    // Второй вход клапана моделирует ложный ТЦ
    // Выход клапана подключен через тройники к повторителям давления тележек
    bc_switch_valve->setInputFlow1(loco_crane->getBCflow());
    bc_switch_valve->setInputFlow2(electro_air_dist->getBCflow());
    bc_switch_valve->setOutputPressure(bc_splitter[0]->getInputPressure());
    bc_switch_valve->step(t, dt);

    // Тройники
    bc_splitter[0]->setInputFlow(bc_switch_valve->getOutputFlow());
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
