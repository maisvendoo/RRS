#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::stepPneumoBrakeSystem(double t, double dt)
{
    double U_gen = starter_generator->getVoltage();

    main_reservoir->setAirFlow(motor_compressor->getAirFlow());
    main_reservoir->step(t, dt);

    // Состояние цепи реле РУ18
    bool is_RU18_on = azv_motor_compressor.getState() &&
                    static_cast<double>(press_reg->getState());

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

    bool is_MK_on = ktk1->getContactState(1);

    ktk2->setVoltage(Ucc * static_cast<double>(rv6->getContactState(0)));
    ktk2->step(t, dt);

    motor_compressor->setU(U_gen * static_cast<double>(is_MK_on));
    motor_compressor->setKontaktorState(0, ktk2->getContactState(0));
    motor_compressor->setPressure(main_reservoir->getPressure());
    motor_compressor->step(t, dt);

    press_reg->setPressure(main_reservoir->getPressure());
    press_reg->step(t, dt);

    // Работа тормозов

    ubt367m->setLocoFLpressure(main_reservoir->getPressure());
    ubt367m->setCraneTMpressure(krm->getBrakePipeInitPressure());
    ubt367m->setControl(keys);
    p0 = ubt367m->getLocoTMpressure();
    ubt367m->step(t, dt);

    krm->setFeedLinePressure(ubt367m->getCraneFLpressure());
    krm->setChargePressure(0.5);
    krm->setBrakePipePressure(pTM);
    krm->setControl(keys);
    krm->step(t, dt);

    kvt->setFeedlinePressure(ubt367m->getCraneFLpressure());
    kvt->setBrakeCylinderPressure(zpk->getPressure2());
    kvt->setAirDistributorFlow(0.0);
    kvt->setControl(keys);
    kvt->step(t, dt);

    zpk->setInputFlow1(evr->getQbc_out());
    zpk->setInputFlow2(kvt->getBrakeCylinderFlow());
    zpk->setOutputPressure(pneumo_splitter->getP_in());
    zpk->step(t, dt);

    pneumo_splitter->setQ_in(zpk->getOutputFlow());
    pneumo_splitter->setP_out1(rd304->getWorkPressure());
    pneumo_splitter->setP_out2(bwd_trolley->getBrakeCylinderPressure());
    pneumo_splitter->step(t, dt);

    rd304->setPipelinePressure(main_reservoir->getPressure());
    rd304->setWorkAirFlow(pneumo_splitter->getQ_out1());
    rd304->setBrakeCylPressure(fwd_trolley->getBrakeCylinderPressure());
    rd304->step(t, dt);

    fwd_trolley->setAirFlow(rd304->getBrakeCylAirFlow());
    fwd_trolley->setVelocity(velocity);
    fwd_trolley->step(t, dt);

    bwd_trolley->setAirFlow(pneumo_splitter->getQ_out2());
    bwd_trolley->setVelocity(velocity);
    bwd_trolley->step(t, dt);

    zr->setAirFlow(evr->getOutputSupplyReservoirFlow());
    zr->step(t, dt);

    vr->setBrakeCylinderPressure(evr->getPbc_out());
    vr->setAirSupplyPressure(evr->getSupplyReservoirPressure());
    vr->setBrakepipePressure(pTM);
    vr->step(t, dt);

    auxRate = vr->getAuxRate();

    evr->setControlLine(ept_control[0]);
    evr->setQbc_in(vr->getBrakeCylinderAirFlow());
    evr->setPbc_in(zpk->getPressure1());
    evr->setInputSupplyReservoirFlow(vr->getAirSupplyFlow());
    evr->setSupplyReservoirPressure(zr->getPressure());
    evr->step(t, dt);
}
