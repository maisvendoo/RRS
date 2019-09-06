#include    "chs2t-brake-mech.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CHS2tBrakeMech::CHS2tBrakeMech(QObject *parent)
    : BrakeMech(parent)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CHS2tBrakeMech::~CHS2tBrakeMech()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2tBrakeMech::load_config(CfgReader &cfg)
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
