#include    "filesystem.h"

#include    "tep70.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void TEP70::initBrakesControl(QString modules_dir)
{
    // Блокировочное устройство
    brake_lock = new BrakeLock();
    brake_lock->read_config("ubt367m");

    // Поездной кран машиниста
    brake_crane = loadBrakeCrane(
                modules_dir + QDir::separator() + brake_crane_module_name);
    brake_crane->read_config(brake_crane_config_name);

    // Кран вспомогательного тормоза
    loco_crane = loadLocoCrane(
                modules_dir + QDir::separator() + loco_crane_module_name);
    loco_crane->read_config(loco_crane_config_name);

    // ЭПК
    epk = loadAutoTrainStop(modules_dir + QDir::separator() + "epk150");
    epk->read_config("epk150");

    // Переключательный клапан магистрали тормозных цилиндров
    bc_switch_valve = new SwitchingValve();
    bc_switch_valve->read_custom_config(config_dir + QDir::separator() + "zpk");

    // Тройник
    bc_splitter = new PneumoSplitter();
    bc_splitter->read_config("pneumo-splitter");

    // Повторительное реле давления
    bc_pressure_relay[TROLLEY_FWD] = new PneumoRelay();
    bc_pressure_relay[TROLLEY_FWD]->read_config("rd304");
    bc_pressure_relay[TROLLEY_BWD] = new PneumoRelay();
    bc_pressure_relay[TROLLEY_BWD]->read_config("rd304");

    // Концевые краны магистрали тормозных цилиндров
    anglecock_bc_fwd = new PneumoAngleCock();
    anglecock_bc_fwd->read_config("pneumo-anglecock-BC");

    anglecock_bc_bwd = new PneumoAngleCock();
    anglecock_bc_bwd->read_config("pneumo-anglecock-BC");

    // Рукава магистрали тормозных цилиндров
    hose_bc_fwd = new PneumoHose();
    hose_bc_fwd->read_config("pneumo-hose-BC");
    forward_connectors.push_back(hose_bc_fwd);

    hose_bc_bwd = new PneumoHose();
    hose_bc_bwd->read_config("pneumo-hose-BC");
    backward_connectors.push_back(hose_bc_bwd);
}
