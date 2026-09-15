#include    "ep1m.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void EP1m::stepPneumoSupply(const double& t, const double& dt)
{
    // Регулятор давления
    press_reg->setFLpressure(main_reservoir->getPressure());
    press_reg->step(t, dt);

    // Цепь включения главного компрессора
    bool button_compr = tumblers[BUTTON_COMPRESSOR][CAB1].getState() ||
                        tumblers[BUTTON_COMPRESSOR][CAB2].getState();
    bool tumbler_compr = tumblers_panel[CAB1]->getTumblerState(EP1MTumblersPanel::TUMBLER_COMPRESSOR) ||
                         tumblers_panel[CAB2]->getTumblerState(EP1MTumblersPanel::TUMBLER_COMPRESSOR);
    double is_mk_On = static_cast<double>(button_compr ||
                                          press_reg->getState() && tumbler_compr);
    double U_power = is_mk_On * freq_phase_conv->getOutputVoltage();

    // Мотор-компрессор
    motor_compressor->setFLpressure(main_reservoir->getPressure());
    motor_compressor->setPowerVoltage(U_power);
    motor_compressor->step(t, dt);

    // Главный резервуар
    double FL_flow = 0.0;
    FL_flow += motor_compressor->getFLflow();
    FL_flow += horn[CAB1]->getFLflow() + horn[CAB2]->getFLflow();
    FL_flow += sand_system->getFLflow();
    FL_flow += brake_lock[CAB1]->getFLflow();
    FL_flow += brake_lock[CAB2]->getFLflow();
    FL_flow += epk[CAB1]->getFLflow();
    FL_flow += epk[CAB2]->getFLflow();
    FL_flow += pneumo_red_panel->getFLflow();
    FL_flow += bc_pressure_relay[TROLLEY_FWD]->getFLflow();
    FL_flow += bc_pressure_relay[TROLLEY_MID]->getFLflow();
    FL_flow += bc_pressure_relay[TROLLEY_BWD]->getFLflow();

    anglecock_fl_fwd->setHoseFlow(hose_fl_fwd->getFlow());
    FL_flow += anglecock_fl_fwd->getFlowToPipe();

    anglecock_fl_bwd->setHoseFlow(hose_fl_bwd->getFlow());
    FL_flow += anglecock_fl_bwd->getFlowToPipe();

    main_reservoir->setFlow(FL_flow);
    main_reservoir->step(t, dt);

    // Концевые краны питательной магистрали
    anglecock_fl_fwd->setPipePressure(main_reservoir->getPressure());
    anglecock_fl_fwd->step(t, dt);
    anglecock_fl_bwd->setPipePressure(main_reservoir->getPressure());
    anglecock_fl_bwd->step(t, dt);

    // Рукава питательной магистрали
    hose_fl_fwd->setPressure(anglecock_fl_fwd->getPressureToHose());
    hose_fl_fwd->setFlowCoeff(anglecock_fl_fwd->getFlowCoeff());
    hose_fl_fwd->setCoord(train_coord + dir * (length / 2.0 - anglecock_fl_fwd->getShiftCoord()));
    hose_fl_fwd->setShiftSide(anglecock_fl_fwd->getShiftSide());
    hose_fl_fwd->step(t, dt);
    hose_fl_bwd->setPressure(anglecock_fl_bwd->getPressureToHose());
    hose_fl_bwd->setFlowCoeff(anglecock_fl_bwd->getFlowCoeff());
    hose_fl_bwd->setCoord(train_coord - dir * (length / 2.0 - anglecock_fl_bwd->getShiftCoord()));
    hose_fl_bwd->setShiftSide(anglecock_fl_bwd->getShiftSide());
    hose_fl_bwd->step(t, dt);
}
