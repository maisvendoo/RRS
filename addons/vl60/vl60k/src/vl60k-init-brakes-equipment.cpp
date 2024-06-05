#include    "filesystem.h"

#include    "vl60k.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60k::initBrakesEquipment(QString modules_dir)
{
    // Тормозная магистраль
    double volume_bp = length * 0.0343 * 0.0343 * Physics::PI / 4.0;
    brakepipe = new Reservoir(volume_bp);
    brakepipe->setLeakCoeff(3e-6);

    // Воздухораспределитель
    air_dist = loadAirDistributor(modules_dir + QDir::separator() + airdist_module_name);
    air_dist->read_config(airdist_config_name);

    // Запасный резервуар
    supply_reservoir = new Reservoir(0.055);
    supply_reservoir->setLeakCoeff(1e-6);

    // Тормозные рычажные передачи
    brake_mech[TROLLEY_FWD] = new BrakeMech(NUM_AXIS_PER_TROLLEY);
    brake_mech[TROLLEY_FWD]->read_custom_config(
                config_dir + QDir::separator() + "brake-mech-fwd");
    brake_mech[TROLLEY_FWD]->setWheelRadius(rk[0]);
    brake_mech[TROLLEY_FWD]->setEffFricRadius(rk[0]);

    brake_mech[TROLLEY_BWD] = new BrakeMech(NUM_AXIS_PER_TROLLEY);
    brake_mech[TROLLEY_BWD]->read_custom_config(
                config_dir + QDir::separator() + "brake-mech-bwd");
    brake_mech[TROLLEY_BWD]->setWheelRadius(rk[NUM_AXIS_PER_TROLLEY]);
    brake_mech[TROLLEY_BWD]->setEffFricRadius(rk[NUM_AXIS_PER_TROLLEY]);

    // Концевые краны тормозной магистрали
    anglecock_bp_fwd = new PneumoAngleCock();
    anglecock_bp_fwd->setKeyCode(KEY_9);
    anglecock_bp_fwd->read_config("pneumo-anglecock-BP");
    anglecock_bp_fwd->setPipeVolume(volume_bp);

    anglecock_bp_bwd = new PneumoAngleCock();
    anglecock_bp_bwd->setKeyCode(KEY_0);
    anglecock_bp_bwd->read_config("pneumo-anglecock-BP");
    anglecock_bp_bwd->setPipeVolume(volume_bp);

    // Рукава тормозной магистрали
    hose_bp_fwd = new PneumoHose();
    //hose_bp_fwd->setKeyCode(0);
    hose_bp_fwd->read_config("pneumo-hose-BP");
    forward_connectors.push_back(hose_bp_fwd);

    hose_bp_bwd = new PneumoHose();
    //hose_bp_bwd->setKeyCode(0);
    hose_bp_bwd->read_config("pneumo-hose-BP");
    backward_connectors.push_back(hose_bp_bwd);
}
