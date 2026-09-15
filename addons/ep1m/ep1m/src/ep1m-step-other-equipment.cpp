#include    "ep1m.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::stepOtherEquipment(const double& t, const double& dt)
{
    for (auto i : {CAB1, CAB2})
    {
        horn[i]->setFLpressure(main_reservoir->getPressure());
        horn[i]->step(t, dt);
    }

    // Система подачи песка
    sand_system->setFLpressure(main_reservoir->getPressure());
    //sand_system->setSandDeliveryOn(button_sandbox.getState());
    sand_system->step(t, dt);
    for (size_t i = 0; i < num_axis; ++i)
    {
        // Пересчёт трения колесо-рельс
        psi[i] = sand_system->getWheelRailFrictionCoeff(psi[i]);
    }
    // Пересчёт массы локомотива
    payload_coeff = sand_system->getSandLevel();
    setPayloadCoeff(payload_coeff);
}
