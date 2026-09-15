#include    "ep1m.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::stepSafetyDevices(const double& t, const double& dt)
{
    // Приёмные катушки АЛСН
    coil_ALSN_fwd->step(t, dt);
    coil_ALSN_bwd->step(t, dt);

    // Дешифратор АЛСН
    alsn_decoder[CAB1]->setCoilSignal(coil_ALSN_fwd->getCode());
    alsn_decoder[CAB1]->step(t, dt);

    alsn_decoder[CAB2]->setCoilSignal(coil_ALSN_bwd->getCode());
    alsn_decoder[CAB2]->step(t, dt);

    // КЛУБ
    bool epk_on = false;
    int direction = dir;
    auto cab_idx = CAB1;
    auto wheel_idx = TRAC_MOTOR1;
    if (epk[CAB1]->isKeyOn())
    {
        klub_BEL->setSpeedMapModule(speedmap_fwd);
        klub_BEL->setCoilALSNModule(coil_ALSN_fwd);
        klub_BEL->setAlsnCode(alsn_decoder[CAB1]->getCode());

        epk_on = true;
    }
    if (epk[CAB2]->isKeyOn())
    {
        klub_BEL->setSpeedMapModule(speedmap_bwd);
        klub_BEL->setCoilALSNModule(coil_ALSN_bwd);
        klub_BEL->setAlsnCode(alsn_decoder[CAB2]->getCode());

        epk_on = true;
        direction *= -1;
        cab_idx = CAB2;
        wheel_idx = TRAC_MOTOR6;
    }

    if (!epk[cab_idx]->isKeyOn())
    {
        shunting_mode_switcher[cab_idx].reset();
    }

    klub_BEL->setVoltage(Ucc);
    klub_BEL->setKeyEPK(epk_on);
    klub_BEL->setCoord(profile_point_data.position);
    klub_BEL->setRailCoord(profile_point_data.railway_coord);
    klub_BEL->setVelocity(wheel_omega[wheel_idx] * wheel_diameter[wheel_idx] / 2.0);
    klub_BEL->setTrainLength(length);
    klub_BEL->setRBstate(epk_on && tumblers[BUTTON_RB][cab_idx].getState());
    klub_BEL->setRBSstate(epk_on && tumblers[BUTTON_RBS][cab_idx].getState());
    klub_BEL->setShuntingMode(shunting_mode_switcher[cab_idx].getState());
    klub_BEL->step(t, dt);
}
