#include    "ep1m.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void EP1m::initPneumoSupply(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) modules_dir;

    // Регулятор давления в ГР
    press_reg = new PressureRegulator();
    press_reg->read_config("pressure-regulator");

    // Мотор-компрессор
    motor_compressor = new ACMotorCompressor();
    motor_compressor->read_config("motor-compressor", custom_cfg_dir);

    // Главный резервуар объемом 1020 литров
    double volume_main = 1.02;
    main_reservoir = new Reservoir(volume_main);

    // Концевые краны питательной магистрали
    anglecock_fl_fwd = new PneumoAngleCock();
    anglecock_fl_fwd->read_config("pneumo-anglecock-FL");

    anglecock_fl_bwd = new PneumoAngleCock();
    anglecock_fl_bwd->read_config("pneumo-anglecock-FL");

    // Рукава питательной магистрали
    hose_fl_fwd = new PneumoHose();
    hose_fl_fwd->read_config("pneumo-hose-FL");
    forward_connectors.push_back(hose_fl_fwd);

    hose_fl_bwd = new PneumoHose();
    hose_fl_bwd->read_config("pneumo-hose-FL");
    backward_connectors.push_back(hose_fl_bwd);
}
