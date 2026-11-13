#include    "vl60pk.h"

#include    <QDir>

#include "brake-crane.h"
#include "loco-crane.h"
#include "pneumo-brake-lock.h"
#include "pneumo-relay.h"
#include "pneumo-switching-valve.h"
#include "core/load_module.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60pk::initBrakesControl(const QString& modules_dir, const QString& custom_cfg_dir)
{
    for (size_t cab_idx : {CAB1, CAB2})
    {
        // Блокировочное устройство
        brake_lock[cab_idx] = new PneumoBrakeLock();
        brake_lock[cab_idx]->read_config("ubt367m");

        // Поездной кран машиниста
        brake_crane[cab_idx] = LOAD_MODULE(BrakeCrane,
            modules_dir + QDir::separator() + brake_crane_module_name);
        brake_crane[cab_idx]->read_config(brake_crane_config_name);

        // Кран вспомогательного тормоза
        loco_crane[cab_idx] = LOAD_MODULE(LocoCrane,
            modules_dir + QDir::separator() + loco_crane_module_name);
        loco_crane[cab_idx]->read_config(loco_crane_config_name);
    }

    // Переключательный клапан магистрали тормозных цилиндров
    bc_switch_valve = new SwitchingValve();
    bc_switch_valve->read_config("zpk", custom_cfg_dir);

    // Повторительное реле давления
    bc_pressure_relay = new PneumoRelay();
    bc_pressure_relay->read_config("rd304");
}
