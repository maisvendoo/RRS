#include    "ep1m.h"

#include    "core/load_module.h"

#include    <QDir>

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void EP1m::initBrakesEquipment(const QString& modules_dir, const QString& custom_cfg_dir)
{
    // Тормозная магистраль
    double volume_bp = length * 0.0343 * 0.0343 * Physics::PI / 4.0;
    brakepipe = new Reservoir(volume_bp);
    brakepipe->setLeakCoeff(3e-6);

    // Воздухораспределитель
    air_dist = LOAD_MODULE(AirDistributor, modules_dir + QDir::separator() + "vr242");
    air_dist->read_config("vr242");

    // Электровоздухораспределитель
    electro_air_dist = LOAD_MODULE(ElectroAirDistributor,
        modules_dir + QDir::separator() + "evr305");
    electro_air_dist->read_config("evr305");

    // Запасный резервуар
    supply_reservoir = new Reservoir(0.055);
    supply_reservoir->setLeakCoeff(1e-6);

    // Тормозные рычажные передачи
    brake_mech[TROLLEY_FWD] = new BrakeMech(NUM_AXIS_PER_TROLLEY);
    brake_mech[TROLLEY_FWD]->read_config("brake-mech-fwd", custom_cfg_dir);
    brake_mech[TROLLEY_FWD]->setWheelRadius(rk[0]);
    brake_mech[TROLLEY_FWD]->setEffFricRadius(rk[0]);

    brake_mech[TROLLEY_MID] = new BrakeMech(NUM_AXIS_PER_TROLLEY);
    brake_mech[TROLLEY_MID]->read_config("brake-mech-mid", custom_cfg_dir);
    brake_mech[TROLLEY_MID]->setWheelRadius(rk[NUM_AXIS_PER_TROLLEY]);
    brake_mech[TROLLEY_MID]->setEffFricRadius(rk[NUM_AXIS_PER_TROLLEY]);

    brake_mech[TROLLEY_BWD] = new BrakeMech(NUM_AXIS_PER_TROLLEY);
    brake_mech[TROLLEY_BWD]->read_config("brake-mech-bwd", custom_cfg_dir);
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
    hose_bp_fwd = LOAD_MODULE(PneumoHoseEPB,
        modules_dir + QDir::separator() + "hose369a");
    hose_bp_fwd->read_config("pneumo-hose-BP369a-loco");
    forward_connectors.push_back(hose_bp_fwd);

    hose_bp_bwd = LOAD_MODULE(PneumoHoseEPB,
        modules_dir + QDir::separator() + "hose369a");
    hose_bp_bwd->read_config("pneumo-hose-BP369a-loco");
    backward_connectors.push_back(hose_bp_bwd);
}
