#include    "filesystem.h"

#include    "ep20.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void EP20::initBrakesEquipment(QString modules_dir)
{
    // Тормозная магистраль
    double volume_bp = length * 0.0343 * 0.0343 * Physics::PI / 4.0;
    brakepipe = new Reservoir(volume_bp);
    brakepipe->setLeakCoeff(3e-6);

    // Воздухораспределитель
    air_dist = loadAirDistributor(modules_dir + QDir::separator() + "vr242");
    air_dist->read_config("vr242");

    // Электровоздухораспределитель
    electro_air_dist = loadElectroAirDistributor(modules_dir + QDir::separator() + "evr305");
    electro_air_dist->read_config("evr305");

    // Запасный резервуар
    supply_reservoir = new Reservoir(0.078);
    supply_reservoir->setLeakCoeff(1e-6);

    // Тормозные рычажные передачи
    brake_mech[TROLLEY_FWD] = new BrakeMech(NUM_AXIS_PER_TROLLEY);
    brake_mech[TROLLEY_FWD]->read_custom_config(
                config_dir + QDir::separator() + "brake-mech-fwd");
    brake_mech[TROLLEY_FWD]->setWheelRadius(rk[0]);
    brake_mech[TROLLEY_FWD]->setEffFricRadius(rk[0]);

    brake_mech[TROLLEY_MID] = new BrakeMech(NUM_AXIS_PER_TROLLEY);
    brake_mech[TROLLEY_MID]->read_custom_config(
                config_dir + QDir::separator() + "brake-mech-mid");
    brake_mech[TROLLEY_MID]->setWheelRadius(rk[NUM_AXIS_PER_TROLLEY]);
    brake_mech[TROLLEY_MID]->setEffFricRadius(rk[NUM_AXIS_PER_TROLLEY]);

    brake_mech[TROLLEY_BWD] = new BrakeMech(NUM_AXIS_PER_TROLLEY);
    brake_mech[TROLLEY_BWD]->read_custom_config(
                config_dir + QDir::separator() + "brake-mech-bwd");
    brake_mech[TROLLEY_BWD]->setWheelRadius(rk[2 * NUM_AXIS_PER_TROLLEY]);
    brake_mech[TROLLEY_BWD]->setEffFricRadius(rk[2 * NUM_AXIS_PER_TROLLEY]);

    // Концевые краны тормозной магистрали
    anglecock_bp_fwd = new PneumoAngleCock();
    anglecock_bp_fwd->read_config("pneumo-anglecock-BP");
    anglecock_bp_fwd->setPipeVolume(volume_bp);

    anglecock_bp_bwd = new PneumoAngleCock();
    anglecock_bp_bwd->read_config("pneumo-anglecock-BP");
    anglecock_bp_bwd->setPipeVolume(volume_bp);

    // Рукава тормозной магистрали
    hose_bp_fwd = loadPneumoHoseEPB(modules_dir + QDir::separator() + "hose369a");
    hose_bp_fwd->read_config("pneumo-hose-BP369a-loco");
    forward_connectors.push_back(hose_bp_fwd);

    hose_bp_bwd = loadPneumoHoseEPB(modules_dir + QDir::separator() + "hose369a");
    hose_bp_bwd->read_config("pneumo-hose-BP369a-loco");
    backward_connectors.push_back(hose_bp_bwd);
}
