#include    "vl60pk.h"

#include    <QDir>

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60pk::initBrakesControl(const QString &modules_dir, const QString &custom_cfg_dir)
{
    // Блокировочное устройство
    brake_lock = new BrakeLock();
    brake_lock->read_config("ubt367m");
    connect(brake_lock, &BrakeLock::soundPlay, this, &VL60pk::soundPlay);

    // Поездной кран машиниста
    brake_crane = loadBrakeCrane(
                modules_dir + QDir::separator() + brake_crane_module_name);
    brake_crane->read_config(brake_crane_config_name);
    connect(brake_crane, &BrakeCrane::soundPlay, this, &VL60pk::soundPlay);
    connect(brake_crane, &BrakeCrane::soundSetVolume, this, &VL60pk::soundSetVolume);

    // Кран вспомогательного тормоза
    loco_crane = loadLocoCrane(
                modules_dir + QDir::separator() + loco_crane_module_name);
    loco_crane->read_config(loco_crane_config_name);
    connect(loco_crane, &LocoCrane::soundPlay, this, &VL60pk::soundPlay);
    connect(loco_crane, &LocoCrane::soundStop, this, &VL60pk::soundStop);
    connect(loco_crane, &LocoCrane::soundSetVolume, this, &VL60pk::soundSetVolume);

    // Переключательный клапан магистрали тормозных цилиндров
    bc_switch_valve = new SwitchingValve();
    bc_switch_valve->read_config("zpk", custom_cfg_dir);

    // Повторительное реле давления
    bc_pressure_relay = new PneumoRelay();
    bc_pressure_relay->read_config("rd304");

    // Концевые краны магистрали тормозных цилиндров
    anglecock_bc_fwd = new PneumoAngleCock();
    //anglecock_bc_fwd->setKeyCode(0);
    anglecock_bc_fwd->read_config("pneumo-anglecock-BC");

    anglecock_bc_bwd = new PneumoAngleCock();
    //anglecock_bc_bwd->setKeyCode(0);
    anglecock_bc_bwd->read_config("pneumo-anglecock-BC");

    // Рукава магистрали тормозных цилиндров
    hose_bc_fwd = new PneumoHose();
    //hose_bc_fwd->setKeyCode(0);
    hose_bc_fwd->read_config("pneumo-hose-BC");
    forward_connectors.push_back(hose_bc_fwd);

    hose_bc_bwd = new PneumoHose();
    //hose_bc_bwd->setKeyCode(0);
    hose_bc_bwd->read_config("pneumo-hose-BC");
    backward_connectors.push_back(hose_bc_bwd);
}
