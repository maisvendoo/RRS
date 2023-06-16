#include    "filesystem.h"

#include    "tep70.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void TEP70::initPneumoSupply(QString modules_dir)
{
    Q_UNUSED(modules_dir)

    // Регулятор давления в ГР
    press_reg = new PressureRegulator();
    press_reg->read_config("pressure-regulator");

    // Мотор-компрессор
    motor_compressor = new TEP70MotorCompressor();
    motor_compressor->read_custom_config(config_dir + QDir::separator() + "motor-compressor");

    // Главный резервуар
    double volume_main = 1.0;
    main_reservoir = new Reservoir(volume_main);
    main_reservoir->setLeakCoeff(1e-6);

    // Элементы цепи управления компрессором
    ru18 = new Relay(3);
    ru18->read_custom_config(config_dir + QDir::separator() + "rpu-3m");
    ru18->setInitContactState(0, false);
    ru18->setInitContactState(1, false);
    ru18->setInitContactState(2, false);

    rv6 = new TimeRelay(1);
    rv6->read_custom_config(config_dir + QDir::separator() + "rpu-3m");
    rv6->setInitContactState(0, false);
    rv6->setTimeout(5.0);

    ktk1 = new Relay(3);
    ktk1->read_custom_config(config_dir + QDir::separator() + "rpu-3m");
    ktk1->setInitContactState(0, false);
    ktk1->setInitContactState(1, false);
    ktk1->setInitContactState(2, false);

    ktk2 = new Relay(1);
    ktk2->read_custom_config(config_dir + QDir::separator() + "rpu-3m");
    ktk2->setInitContactState(0, false);

    azv_motor_compressor.set();

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
