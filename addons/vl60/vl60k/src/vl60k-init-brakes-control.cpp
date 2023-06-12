#include    "filesystem.h"

#include    "vl60k.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60k::initBrakesControl(QString modules_dir)
{
    // Блокировочное устройство
    brake_lock = new BrakeLock();
    brake_lock->read_config("ubt367m");
    connect(brake_lock, &BrakeLock::soundPlay, this, &VL60k::soundPlay);

    // Поездной кран машиниста
    brake_crane = loadBrakeCrane(modules_dir + QDir::separator() + "krm395");
    brake_crane->read_config("krm395");
    connect(brake_crane, &BrakeCrane::soundPlay, this, &VL60k::soundPlay);
    connect(brake_crane, &BrakeCrane::soundSetVolume, this, &VL60k::soundSetVolume);

    // Кран вспомогательного тормоза
    loco_crane = loadLocoCrane(modules_dir + QDir::separator() + "kvt254");
    loco_crane->read_config("kvt254");
    connect(loco_crane, &LocoCrane::soundPlay, this, &VL60k::soundPlay);
    connect(loco_crane, &LocoCrane::soundStop, this, &VL60k::soundStop);
    connect(loco_crane, &LocoCrane::soundSetVolume, this, &VL60k::soundSetVolume);

    // Импульсная магистраль с ложным тормозным цилиндром
    impulse_line = new Reservoir(0.005 + 0.007);

    // Тройник магистрали тормозных цилиндров
    bc_splitter = new PneumoSplitter();
    bc_splitter->read_config("pneumo-splitter");

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
