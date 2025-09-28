#include    "vl60k.h"

#include "brake-crane.h"
#include "loco-crane.h"
#include "pneumo-brake-lock.h"
#include "reservoir.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60k::load_brakes_config(QString path)
{
    CfgReader cfg;

    if (cfg.load(path))
    {
        QString secName = "BrakesState";
        double tmp_dbl;
        int tmp_int;

        tmp_dbl = 1.0e-4;
        if (cfg.getDouble(secName, "MainReservoirLeak", tmp_dbl))
        {
            main_reservoir->setLeakCoeff(tmp_dbl);
        }

        tmp_int = 2;
        if (cfg.getInt(secName, "TrainCranePosCab1", tmp_int))
        {
            brake_crane[CAB1]->setHandlePosition(tmp_int - 1);
        }

        tmp_int = 7;
        if (cfg.getInt(secName, "TrainCranePosCab2", tmp_int))
        {
            brake_crane[CAB2]->setHandlePosition(tmp_int - 1);
        }

        tmp_dbl = 1.0;
        if (cfg.getDouble(secName, "LocoCranePosCab1", tmp_dbl))
        {
            loco_crane[CAB1]->setHandlePosition(tmp_dbl);
        }

        tmp_dbl = 1.0;
        if (cfg.getDouble(secName, "LocoCranePosCab2", tmp_dbl))
        {
            loco_crane[CAB2]->setHandlePosition(tmp_dbl);
        }

        tmp_int = 0;
        if (cfg.getInt(secName, "CombineCranePosCab1", tmp_int))
        {
            brake_lock[CAB1]->setCombineCranePosition(tmp_int);
        }

        tmp_int = -1;
        if (cfg.getInt(secName, "CombineCranePosCab2", tmp_int))
        {
            brake_lock[CAB2]->setCombineCranePosition(tmp_int);
        }

        tmp_int = 1;
        if (cfg.getInt(secName, "BrakeLockDeviceCab1", tmp_int))
        {
            brake_lock[CAB1]->setStateOn(tmp_int);
        }
        // Не допускаем двух рукояток в устройствах блокировки тормозов
        brake_lock[CAB2]->allowLockHandle(!(brake_lock[CAB1]->isLockHandle()));

        tmp_int = 0;
        if (cfg.getInt(secName, "BrakeLockDeviceCab2", tmp_int))
        {
            brake_lock[CAB2]->setStateOn(tmp_int);
        }
        // Не допускаем двух рукояток в устройствах блокировки тормозов
        brake_lock[CAB1]->allowLockHandle(!(brake_lock[CAB2]->isLockHandle()));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60k::loadConfig(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Vehicle";

        cfg.getDouble(secName, "ReductorCoeff", ip);
        cfg.getString(secName, "CouplingModule", coupling_module_name);
        cfg.getString(secName, "CouplingConfig", coupling_config_name);
        cfg.getString(secName, "BrakeCraneModule", brake_crane_module_name);
        cfg.getString(secName, "BrakeCraneConfig", brake_crane_config_name);
        cfg.getString(secName, "LocoCraneModule", loco_crane_module_name);
        cfg.getString(secName, "LocoCraneConfig", loco_crane_config_name);
        cfg.getString(secName, "AirDistModule", airdist_module_name);
        cfg.getString(secName, "AirDistConfig", airdist_config_name);
    }
}
