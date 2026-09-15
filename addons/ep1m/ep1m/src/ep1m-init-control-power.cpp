#include    "ep1m.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::initControlPower(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) modules_dir;

    battery = new Battery();
    battery->read_config("battery", custom_cfg_dir);

    power_supply = new PowerSupply();    

    km5 = new Relay(1);
    km5->read_config("mk-69", custom_cfg_dir);
    km5->setInitContactState(0, false);
}
