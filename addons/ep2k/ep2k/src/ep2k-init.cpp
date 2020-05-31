#include    "ep2k.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP2K::initialization()
{
    initHighVoltageSide();

    initTracDrive();

    reg = new Registrator("ep2k", 0.01);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP2K::initHighVoltageSide()
{
    for (size_t i = 0; i < pantogrph.size(); ++i)
    {
        pantogrph[i] = new Pantograph;       
    }

    pantogrph[PANT_FWD]->read_custom_config(config_dir + QDir::separator() + "pantograph_fwd");
    pantogrph[PANT_BWD]->read_custom_config(config_dir + QDir::separator() + "pantograph_bwd");

    fast_switch = new ProtectiveDevice;
    fast_switch->read_custom_config(config_dir + QDir::separator() + "bv");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP2K::initTracDrive()
{
    for (size_t i = 0; i < motor.size(); ++i)
    {
        motor[i] = new TractionMotor;
        motor[i]->read_custom_config(config_dir + QDir::separator() + "dtk800");
        motor[i]->load_magnetic_char(config_dir + QDir::separator() + "dtk800.txt");

        trac_conv[i] = new DCPulseConverter;
    }

    field_conv = new DCPulseConverter;

    trac_reg = new TractionRegulator;
    trac_reg->read_custom_config(config_dir + QDir::separator() + "trac_reg");

    brake_reg = new EDBRegulator;
    brake_reg->read_custom_config(config_dir + QDir::separator() + "brake_reg");

    km = new DriverController;
}
