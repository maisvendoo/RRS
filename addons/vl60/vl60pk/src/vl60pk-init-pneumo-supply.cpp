#include    "vl60pk.h"

#include "motor-compressor-ac.h"
#include "pneumo-anglecock.h"
#include "pneumo-hose.h"
#include "pressure-regulator.h"
#include "reservoir.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60pk::initPneumoSupply(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) modules_dir;
    (void) custom_cfg_dir;

    // Регулятор давления в ГР
    press_reg = new PressureRegulator();
    press_reg->read_config("pressure-regulator");

    // Мотор-компрессор
    motor_compressor = new ACMotorCompressor();
    motor_compressor->read_config("motor-compressor-ac");

    // Главный резервуар
    double volume_main = static_cast<double>(MAIN_RESERVOIR_VOLUME) / 1000.0;
    main_reservoir = new Reservoir(volume_main);
    main_reservoir->setLeakCoeff(1e-6);

    // Концевые краны питательной магистрали
    anglecock_fl_fwd = new PneumoAngleCock();
    anglecock_fl_fwd->read_config("pneumo-anglecock-FL");

    anglecock_fl_bwd = new PneumoAngleCock();
    anglecock_fl_bwd->read_config("pneumo-anglecock-FL");

    // Рукава питательной магистрали
    dvec3 hose_fl_root_coord = {0.3, length / 2.0 - 0.3, 0.8};

    hose_fl_fwd = new PneumoHose();
    hose_fl_fwd->read_config("pneumo-hose-FL");
    hose_fl_fwd->setRootPosition(hose_fl_root_coord);
    forward_connectors.push_back(hose_fl_fwd);

    hose_fl_bwd = new PneumoHose();
    hose_fl_bwd->read_config("pneumo-hose-FL");
    hose_fl_bwd->setRootPosition(hose_fl_root_coord);
    backward_connectors.push_back(hose_fl_bwd);
}
