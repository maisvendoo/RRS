#include    "filesystem.h"

#include    "ep20.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void EP20::initBrakesControl(QString modules_dir)
{
    // Поездной кран машиниста
    brake_crane = loadBrakeCrane(
                modules_dir + QDir::separator() + brake_crane_module_name);
    brake_crane->read_config(brake_crane_config_name);

    // Кран вспомогательного тормоза
    loco_crane = loadLocoCrane(
                modules_dir + QDir::separator() + loco_crane_module_name);
    loco_crane->read_config(loco_crane_config_name);

    // Переключательный клапан магистрали тормозных цилиндров
    bc_switch_valve = new SwitchingValve();
    bc_switch_valve->read_custom_config(config_dir + QDir::separator() + "zpk");

    // Тройники
    bc_splitter[0] = new PneumoSplitter();
    bc_splitter[0]->read_config("pneumo-splitter");
    bc_splitter[1] = new PneumoSplitter();
    bc_splitter[1]->read_config("pneumo-splitter");

    // Повторительное реле давления
    bc_pressure_relay[TROLLEY_FWD] = new PneumoRelay();
    bc_pressure_relay[TROLLEY_FWD]->read_config("rd304");
    bc_pressure_relay[TROLLEY_MID] = new PneumoRelay();
    bc_pressure_relay[TROLLEY_MID]->read_config("rd304");
    bc_pressure_relay[TROLLEY_BWD] = new PneumoRelay();
    bc_pressure_relay[TROLLEY_BWD]->read_config("rd304");
}
