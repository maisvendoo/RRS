#include    "vl60k.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60k::initPneumoSupply(const QString &modules_dir, const QString &custom_cfg_dir)
{
    (void) modules_dir;
    (void) custom_cfg_dir;

    // Регулятор давления в ГР
    press_reg = new PressureRegulator();
    press_reg->read_config("pressure-regulator");

    // Мотор-компрессор
    motor_compressor = new ACMotorCompressor();
    motor_compressor->read_config("motor-compressor-ac");
    connect(motor_compressor, &ACMotorCompressor::soundPlay, this, &VL60k::soundPlay);
    connect(motor_compressor, &ACMotorCompressor::soundStop, this, &VL60k::soundStop);
    connect(motor_compressor, &ACMotorCompressor::soundSetPitch, this, &VL60k::soundSetPitch);

    // Главный резервуар
    double volume_main = static_cast<double>(MAIN_RESERVOIR_VOLUME) / 1000.0;
    main_reservoir = new Reservoir(volume_main);
    main_reservoir->setLeakCoeff(1e-6);

    // Концевые краны питательной магистрали
    anglecock_fl_fwd = new PneumoAngleCock();
    //anglecock_fl_fwd->setKeyCode(0);
    anglecock_fl_fwd->read_config("pneumo-anglecock-FL");

    anglecock_fl_bwd = new PneumoAngleCock();
    //anglecock_fl_bwd->setKeyCode(0);
    anglecock_fl_bwd->read_config("pneumo-anglecock-FL");

    // Рукава питательной магистрали
    hose_fl_fwd = new PneumoHose();
    //hose_fl_fwd->setKeyCode(0);
    hose_fl_fwd->read_config("pneumo-hose-FL");
    forward_connectors.push_back(hose_fl_fwd);

    hose_fl_bwd = new PneumoHose();
    //hose_fl_bwd->setKeyCode(0);
    hose_fl_bwd->read_config("pneumo-hose-FL");
    backward_connectors.push_back(hose_fl_bwd);
}
