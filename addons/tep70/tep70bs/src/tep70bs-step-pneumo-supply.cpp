#include    "tep70bs.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void TEP70BS::stepPneumoSupply(double t, double dt)
{
    press_reg->setFLpressure(main_reservoir->getPressure());
    press_reg->step(t, dt);

    // Состояние цепи реле РУ18
    bool is_RU18_on = azv_motor_compressor.getState() &&
                    static_cast<bool>(press_reg->getState());

    ru18->setVoltage(Ucc * static_cast<double>(is_RU18_on));
    ru18->step(t, dt);

    bool is_RV6_on = azv_motor_compressor.getState() &&
                     krn->getContactState(4) &&
                     ru18->getContactState(0) &&
                     ktk1->getContactState(0);

    rv6->setControlVoltage(Ucc * static_cast<double>(is_RV6_on));
    rv6->step(t, dt);

    bool is_KTK1_on = azv_motor_compressor.getState() &&
                      krn->getContactState(5) &&
                      ru18->getContactState(1);

    ktk1->setVoltage(Ucc * static_cast<double>(is_KTK1_on));
    ktk1->step(t, dt);

    ktk2->setVoltage(Ucc * static_cast<double>(rv6->getContactState(0)));
    ktk2->step(t, dt);

    bool is_MK_on = ktk1->getContactState(1);
    double U_power = starter_generator->getVoltage()
                   * static_cast<double>(is_MK_on);
    motor_compressor->setFLpressure(main_reservoir->getPressure());
    motor_compressor->setPowerVoltage(U_power);
    motor_compressor->setKontaktorState(0, ktk2->getContactState(0));
    motor_compressor->step(t, dt);

    double FL_flow = 0.0;
    FL_flow += motor_compressor->getFLflow();
    FL_flow += brake_lock->getFLflow();
    FL_flow += epk->getFLflow();
    FL_flow += bc_pressure_relay[TROLLEY_FWD]->getFLflow();
    FL_flow += bc_pressure_relay[TROLLEY_BWD]->getFLflow();
    FL_flow += anglecock_fl_fwd->getFlowToPipe();
    FL_flow += anglecock_fl_bwd->getFlowToPipe();
    main_reservoir->setFlow(FL_flow);
    main_reservoir->step(t, dt);

    // Концевые краны питательной магистрали
    anglecock_fl_fwd->setPipePressure(main_reservoir->getPressure());
    anglecock_fl_fwd->setHoseFlow(hose_fl_fwd->getFlow());
    anglecock_fl_fwd->step(t, dt);
    anglecock_fl_bwd->setPipePressure(main_reservoir->getPressure());
    anglecock_fl_bwd->setHoseFlow(hose_fl_bwd->getFlow());
    anglecock_fl_bwd->step(t, dt);

    // Рукава тормозной питательной магистрали
    hose_fl_fwd->setPressure(anglecock_fl_fwd->getPressureToHose());
    hose_fl_fwd->setFlowCoeff(anglecock_fl_fwd->getFlowCoeff());
    hose_fl_fwd->step(t, dt);
    hose_fl_bwd->setPressure(anglecock_fl_bwd->getPressureToHose());
    hose_fl_bwd->setFlowCoeff(anglecock_fl_bwd->getFlowCoeff());
    hose_fl_bwd->step(t, dt);
}
