#include    "vl60pk.h"

#include    "filesystem.h"

#include "airdistributor.h"
#include "brake-crane.h"
#include "electro-airdistributor.h"
#include "loco-crane.h"
#include "automatic-train-stop.h"
#include "pneumo-brake-lock.h"
#include "pneumo-anglecock.h"
#include "pneumo-hose.h"
#include "pneumo-hose-epb.h"
#include "reservoir.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60pk::initBrakeDevices(double p0, double pBP, double pFL)
{
    // Загрузка состояния тормозного оборудования из собственного конфига
    FileSystem &fs = FileSystem::getInstance();
    QString custom_cfg_dir(fs.getVehiclesDir().c_str());
    custom_cfg_dir += QDir::separator() + config_dir;
    load_brakes_config(custom_cfg_dir + QDir::separator() + "brakes-init.xml");

    // Инициализация давления в приборах управления тормозами
    charge_press = p0;
    for (size_t cab_idx : {CAB1, CAB2})
    {
        brake_lock[cab_idx]->init(pBP, pFL);

        brake_crane[cab_idx]->init(pBP, pFL);
        brake_crane[cab_idx]->setChargePressure(charge_press);

        loco_crane[cab_idx]->init(pBP, pFL);

        epk[cab_idx]->init(pBP, pFL);
    }

    // Инициализация давления в питательной магистрали
    main_reservoir->setY(0, pFL);
    anglecock_fl_fwd->setPipePressure(pFL);
    anglecock_fl_bwd->setPipePressure(pFL);
    hose_fl_fwd->setPressure(pFL);
    hose_fl_bwd->setPressure(pFL);

    // Инициализация давления в тормозной магистрали
    brakepipe->setY(0, pBP);
    anglecock_bp_fwd->setPipePressure(pBP);
    anglecock_bp_bwd->setPipePressure(pBP);
    hose_bp_fwd->setPressure(pBP);
    hose_bp_bwd->setPressure(pBP);

    air_dist->init(pBP, pFL);
    electro_air_dist->init(pBP, pFL);

    supply_reservoir->setY(0, pBP);

    // Состояние рукавов и концевых кранов магистрали тормозных цилиндров
    if (hose_bc_fwd->isLinked())
    {
        hose_bc_fwd->connect();
        anglecock_bc_fwd->open();
    }
    else
    {
        anglecock_bc_fwd->close();
    }

    if (hose_bc_bwd->isLinked())
    {
        hose_bc_bwd->connect();
        anglecock_bc_bwd->open();
    }
    else
    {
        anglecock_bc_bwd->close();
    }

    // Состояние рукавов и концевых кранов питательной магистрали
    if (hose_fl_fwd->isLinked())
    {
        hose_fl_fwd->connect();
        anglecock_fl_fwd->open();
    }
    else
    {
        anglecock_fl_fwd->close();
    }

    if (hose_fl_bwd->isLinked())
    {
        hose_fl_bwd->connect();
        anglecock_fl_bwd->open();
    }
    else
    {
        anglecock_fl_bwd->close();
    }

    // Состояние рукавов и концевых кранов тормозной магистрали
    if (hose_bp_fwd->isLinked())
    {
        hose_bp_fwd->connect();
        anglecock_bp_fwd->open();
    }
    else
    {
        anglecock_bp_fwd->close();
    }

    if (hose_bp_bwd->isLinked())
    {
        hose_bp_bwd->connect();
        anglecock_bp_bwd->open();
    }
    else
    {
        anglecock_bp_bwd->close();
    }
}
