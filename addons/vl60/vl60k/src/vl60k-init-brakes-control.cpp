#include    "vl60k.h"

#include    <QDir>

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60k::initBrakesControl(const QString &modules_dir, const QString &custom_cfg_dir)
{
    (void) modules_dir;

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

    // Импульсная магистраль с ложным тормозным цилиндром
    impulse_line = new Reservoir(0.005 + 0.007);

    // Тройник магистрали тормозных цилиндров
    bc_splitter = new PneumoSplitter();
    bc_splitter->read_config("bc-splitter", custom_cfg_dir);

    // Концевые краны магистрали тормозных цилиндров
    anglecock_bc_fwd = new PneumoAngleCock();
    anglecock_bc_fwd->setKeyCode(KEY_F10);
    anglecock_bc_fwd->read_config("pneumo-anglecock-BC");

    anglecock_bc_bwd = new PneumoAngleCock();
    anglecock_bc_bwd->setKeyCode(KEY_F11);
    anglecock_bc_bwd->read_config("pneumo-anglecock-BC");

    // Рукава магистрали тормозных цилиндров
    hose_bc_fwd = new PneumoHose();
    hose_bc_fwd->setKeyCode(KEY_F9);
    hose_bc_fwd->read_config("pneumo-hose-BC");
    forward_connectors.push_back(hose_bc_fwd);

    hose_bc_bwd = new PneumoHose();
    hose_bc_bwd->setKeyCode(KEY_F12);
    hose_bc_bwd->read_config("pneumo-hose-BC");
    backward_connectors.push_back(hose_bc_bwd);
}
