#include    "vl60k.h"

#include    <QDir>

#include "brake-crane.h"
#include "loco-crane.h"
#include "pneumo-brake-lock.h"
#include "pneumo-anglecock.h"
#include "pneumo-hose.h"
#include "pneumo-splitter.h"
#include "reservoir.h"
#include "core/load_module.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60k::initBrakesControl(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) modules_dir;

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
        loco_crane[cab_idx] = loadLocoCrane(
            modules_dir + QDir::separator() + loco_crane_module_name);
        loco_crane[cab_idx]->read_config(loco_crane_config_name);
    }

    // Импульсная магистраль с ложным тормозным цилиндром
    impulse_line = new Reservoir(0.005 + 0.007);

    // Тройник магистрали тормозных цилиндров
    bc_splitter = new PneumoSplitter();
    bc_splitter->read_config("bc-splitter", custom_cfg_dir);

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
