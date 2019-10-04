#include    "vl60.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::load_brakes_config(QString path)
{
    CfgReader cfg;

    if (cfg.load(path))
    {
        QString secName = "BrakesState";

        double pFL = 0.0;

        if (cfg.getDouble(secName, "MainReservoirPressure", pFL))
        {
            main_reservoir->setY(0, pFL);
        }

        double k_flow = 0.0;

        if (cfg.getDouble(secName, "MainReservoirFlow", k_flow))
        {
            main_reservoir->setFlowCoeff(k_flow);
        }

        double ch_press = 0.0;

        if (cfg.getDouble(secName, "ChargingPressure", ch_press))
        {
            charge_press = ch_press;
        }

        int train_crane_pos = 6;

        if (cfg.getInt(secName, "TrainCranePos", train_crane_pos))
        {
            brake_crane->setPosition(train_crane_pos);
        }

        int loco_crane_pos = 0;

        if (cfg.getInt(secName, "LocoCranePos", loco_crane_pos))
        {
            loco_crane->setHandlePosition(loco_crane_pos);
        }

        int brake_lock = 0;

        int combine_crane_pos = -1;

        if (cfg.getInt(secName, "CombineCranePos", combine_crane_pos))
        {
            ubt->setCombineCranePos(combine_crane_pos);
        }

        if (cfg.getInt(secName, "BrakeLockDevice", brake_lock))
        {
            ubt->setState(brake_lock);

            if (brake_lock == 1)
            {
                ubt->setY(0, charge_press);
                brake_crane->init(charge_press, pFL);
                supply_reservoir->setY(0, charge_press);
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::loadConfig(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Vehicle";

        cfg.getDouble(secName, "ReductorCoeff", ip);
    }
}
