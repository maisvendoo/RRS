#include    "filesystem.h"

#include    "passcar.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void PassCar::initBrakesEquipment(QString modules_dir)
{
    // Тормозная магистраль
    double volume_bp = length * 0.0343 * 0.0343 * Physics::PI / 4.0;
    brakepipe = new Reservoir(volume_bp);
    brakepipe->setLeakCoeff(bp_leak);

    // Воздухораспределитель
    air_dist = loadAirDistributor(
                modules_dir + QDir::separator() + air_dist_module);
    air_dist->read_config(air_dist_config);

    // Электровоздухораспределитель
    electro_air_dist = loadElectroAirDistributor(
                modules_dir + QDir::separator() + electro_air_dist_module);
    if (electro_air_dist != nullptr)
        electro_air_dist->read_config(electro_air_dist_config);

    // Запасный резервуар
    supply_reservoir = new Reservoir(sr_volume);
    supply_reservoir->setLeakCoeff(sr_leak);

    // Тормозные рычажные передачи
    brake_mech = new BrakeMech(num_axis);
    brake_mech->setEffFricRadius(wheel_diameter[0] / 2.0);
    brake_mech->setWheelRadius(wheel_diameter[0] / 2.0);
    brake_mech->read_config(brake_mech_config);

    // Концевые краны тормозной магистрали
    anglecock_bp_fwd = new PneumoAngleCock();
    anglecock_bp_fwd->read_config(anglecock_bp_config);
    anglecock_bp_fwd->setPipeVolume(volume_bp);

    anglecock_bp_bwd = new PneumoAngleCock();
    anglecock_bp_bwd->read_config(anglecock_bp_config);
    anglecock_bp_bwd->setPipeVolume(volume_bp);

    // Рукава тормозной магистрали
    hose_bp_fwd = loadPneumoHoseEPB(
                modules_dir + QDir::separator() + hose_bp_module);
    if (hose_bp_fwd == nullptr)
        hose_bp_fwd = new PneumoHoseEPB();
    hose_bp_fwd->read_config(hose_bp_config);
    forward_connectors.push_back(hose_bp_fwd);

    hose_bp_bwd = loadPneumoHoseEPB(
                modules_dir + QDir::separator() + hose_bp_module);
    if (hose_bp_bwd == nullptr)
        hose_bp_bwd = new PneumoHoseEPB();
    hose_bp_bwd->read_config(hose_bp_config);
    backward_connectors.push_back(hose_bp_bwd);
}
