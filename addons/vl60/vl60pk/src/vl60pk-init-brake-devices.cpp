#include    "vl60pk.h"

#include    "filesystem.h"

#include    <vl60-controls.h>

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
#include <kme-60-044.h>

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
        initClientInputSignal(CABS_NUM, CTRL_HOSE_BC_FWD, 1.0f);
        initClientInputSignal(CABS_NUM, CTRL_ANGLECOCK_BC_FWD, 1.0f);
    }
    else
    {
        anglecock_bc_fwd->close();
        initClientInputSignal(CABS_NUM, CTRL_HOSE_BC_FWD, 0.0f);
        initClientInputSignal(CABS_NUM, CTRL_ANGLECOCK_BC_FWD, 0.0f);
    }

    if (hose_bc_bwd->isLinked())
    {
        hose_bc_bwd->connect();
        anglecock_bc_bwd->open();
        initClientInputSignal(CABS_NUM, CTRL_HOSE_BC_BWD, 1.0f);
        initClientInputSignal(CABS_NUM, CTRL_ANGLECOCK_BC_BWD, 1.0f);
    }
    else
    {
        anglecock_bc_bwd->close();
        initClientInputSignal(CABS_NUM, CTRL_HOSE_BC_BWD, 0.0f);
        initClientInputSignal(CABS_NUM, CTRL_ANGLECOCK_BC_BWD, 0.0f);
    }

    // Состояние рукавов и концевых кранов питательной магистрали
    if (hose_fl_fwd->isLinked())
    {
        hose_fl_fwd->connect();
        anglecock_fl_fwd->open();
        initClientInputSignal(CABS_NUM, CTRL_HOSE_FL_FWD, 1.0f);
        initClientInputSignal(CABS_NUM, CTRL_ANGLECOCK_FL_FWD, 1.0f);
    }
    else
    {
        anglecock_fl_fwd->close();
        initClientInputSignal(CABS_NUM, CTRL_HOSE_FL_FWD, 0.0f);
        initClientInputSignal(CABS_NUM, CTRL_ANGLECOCK_FL_FWD, 0.0f);
    }

    if (hose_fl_bwd->isLinked())
    {
        hose_fl_bwd->connect();
        anglecock_fl_bwd->open();
        initClientInputSignal(CABS_NUM, CTRL_HOSE_FL_BWD, 1.0f);
        initClientInputSignal(CABS_NUM, CTRL_ANGLECOCK_FL_BWD, 1.0f);
    }
    else
    {
        anglecock_fl_bwd->close();
        initClientInputSignal(CABS_NUM, CTRL_HOSE_FL_BWD, 0.0f);
        initClientInputSignal(CABS_NUM, CTRL_ANGLECOCK_FL_BWD, 0.0f);
    }

    // Состояние рукавов и концевых кранов тормозной магистрали
    if (hose_bp_fwd->isLinked())
    {
        hose_bp_fwd->connect();
        anglecock_bp_fwd->open();
        initClientInputSignal(CABS_NUM, CTRL_HOSE_BP_FWD, 1.0f);
        initClientInputSignal(CABS_NUM, CTRL_ANGLECOCK_BP_FWD, 1.0f);
    }
    else
    {
        anglecock_bp_fwd->close();
        initClientInputSignal(CABS_NUM, CTRL_HOSE_BP_FWD, 0.0f);
        initClientInputSignal(CABS_NUM, CTRL_ANGLECOCK_BP_FWD, 0.0f);
    }

    if (hose_bp_bwd->isLinked())
    {
        hose_bp_bwd->connect();
        anglecock_bp_bwd->open();
        initClientInputSignal(CABS_NUM, CTRL_HOSE_BP_BWD, 1.0f);
        initClientInputSignal(CABS_NUM, CTRL_ANGLECOCK_BP_BWD, 1.0f);
    }
    else
    {
        anglecock_bp_bwd->close();
        initClientInputSignal(CABS_NUM, CTRL_HOSE_BP_BWD, 0.0f);
        initClientInputSignal(CABS_NUM, CTRL_ANGLECOCK_BP_BWD, 0.0f);
    }    
}
