#include    "ep1m.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::initAuxMachines(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) modules_dir;

    freq_phase_conv = new FreqPhaseConverter();

    km7 = new Relay(2);
    km7->read_config("mk-69", custom_cfg_dir);
    km7->setInitContactState(0, false);
    km7->setInitContactState(1, false);

    km8 = new Relay(2);
    km8->read_config("mk-69", custom_cfg_dir);
    km8->setInitContactState(0, false);
    km8->setInitContactState(1, false);

    km9 = new Relay(2);
    km9->read_config("mk-69", custom_cfg_dir);
    km9->setInitContactState(0, false);
    km9->setInitContactState(1, false);

    km11 = new Relay(2);
    km11->read_config("mk-69", custom_cfg_dir);
    km11->setInitContactState(0, false);
    km11->setInitContactState(1, false);

    km12 = new Relay(2);
    km12->read_config("mk-69", custom_cfg_dir);
    km12->setInitContactState(0, false);
    km12->setInitContactState(1, false);

    km13 = new Relay(2);
    km13->read_config("mk-69", custom_cfg_dir);
    km13->setInitContactState(0, false);
    km13->setInitContactState(1, false);

    for (size_t i = 0; i < motor_fan.size(); ++i)
    {
        motor_fan[i] = new MotorFan(i + 1);
    }
}
