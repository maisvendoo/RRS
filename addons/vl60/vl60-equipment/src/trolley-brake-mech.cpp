#include    "trolley-brake-mech.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrolleyBrakeMech::TrolleyBrakeMech(QString cfg_path, QObject *parent)
    : BrakeMech(parent)
{
    load_config(cfg_path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrolleyBrakeMech::~TrolleyBrakeMech()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrolleyBrakeMech::load_config(CfgReader &cfg)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrolleyBrakeMech::load_config(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Device";

        cfg.getString(secName, "ShoeType", shoeType);
        cfg.getInt(secName, "ShoesCyl", shoesCyl);
        cfg.getInt(secName, "ShoesAxis", shoesAxis);
        cfg.getInt(secName, "CylNum", cylNum);
        cfg.getDouble(secName, "DeadVolume", V0);

        cfg.getDouble(secName, "CylinderDiameter", cylDiam);
        S = Physics::PI * cylDiam * cylDiam / 4.0;

        cfg.getDouble(secName, "MechCoeff", ip);
        cfg.getDouble(secName, "StockOut", Lmax);
        cfg.getDouble(secName, "SpringForce", F0);

        double p_cyl_begin = 0.0;
        cfg.getDouble(secName, "InitPressure", p_cyl_begin);

        setY(0, p_cyl_begin);
    }
}
