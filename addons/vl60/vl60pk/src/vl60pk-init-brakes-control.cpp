#include    "vl60pk.h"

#include    <QDir>

#include "brake-crane.h"
#include "brake-lock.h"
#include "loco-crane.h"
#include "pneumo-anglecock.h"
#include "pneumo-hose.h"
#include "pneumo-relay.h"
#include "pneumo-switching-valve.h"
#include    "key-symbols.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60pk::initBrakesControl(const QString& modules_dir, const QString& custom_cfg_dir)
{
    for (size_t cab_idx : {CAB1, CAB2})
    {
        // Блокировочное устройство
        brake_lock[cab_idx] = new BrakeLock();
        brake_lock[cab_idx]->read_config("ubt367m");

        // Поездной кран машиниста
        brake_crane[cab_idx] = loadBrakeCrane(
            modules_dir + QDir::separator() + brake_crane_module_name);
        brake_crane[cab_idx]->read_config(brake_crane_config_name);

        // Кран вспомогательного тормоза
        loco_crane[cab_idx] = loadLocoCrane(
            modules_dir + QDir::separator() + loco_crane_module_name);
        loco_crane[cab_idx]->read_config(loco_crane_config_name);
    }

    // Переключательный клапан магистрали тормозных цилиндров
    bc_switch_valve = new SwitchingValve();
    bc_switch_valve->read_config("zpk", custom_cfg_dir);

    // Повторительное реле давления
    bc_pressure_relay = new PneumoRelay();
    bc_pressure_relay->read_config("rd304");

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
