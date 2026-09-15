#include    "ep1m.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::initOtherEquipment(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) modules_dir;
    (void) custom_cfg_dir;

    for (auto i : {CAB1, CAB2})
    {
        horn[i] = new TrainHorn();
        horn[i]->read_config("train-horn");
    }

    // Система подачи песка
    sand_system = new SandingSystem();
    sand_system->read_config("sanding-system");
    sand_system->setSandMassMax(payload_mass);
    sand_system->setSandLevel(payload_coeff);
}
