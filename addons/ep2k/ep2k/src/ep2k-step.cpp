#include    "ep2k.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP2K::step(double t, double dt)
{
    p0 = 0.5;

    stepHighVoltageSide(t, dt);

    stepTracDrive(t, dt);

    stepSignals(t, dt);

    DebugMsg = QString("t: %1 v: %2 Iя: %3 Iв: %4 Trac: %5")
            .arg(t, 10, 'f', 1)
            .arg(velocity * Physics::kmh, 6, 'f', 1)
            .arg(motor[0]->getAncorCurrent(), 6, 'f', 1)
            .arg(motor[0]->getFieldCurrent(), 6, 'f', 1)
            .arg(km->getPosition(), 6, 'f', 1);

    QString msg = QString("%1 %2 %3 %4 %5 %6")
            .arg(t, 10, 'f', 1)
            .arg(velocity * Physics::kmh, 6, 'f', 1)
            .arg(motor[0]->getAncorCurrent(), 6, 'f', 1)
            .arg(motor[0]->getFieldCurrent(), 6, 'f', 1)
            .arg(trac_conv[0]->getU_out(), 6, 'f', 1)
            .arg(tracForce / 1000.0, 6, 'f', 1);

    reg->print(msg, t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP2K::stepHighVoltageSide(double t, double dt)
{
    for (size_t i = 0; i < pantogrph.size(); ++i)
    {
        pantogrph[i]->setUks(Uks);
        pantogrph[i]->step(t, dt);
    }

    double Ukr = max(pantogrph[PANT_FWD]->getUout(), pantogrph[PANT_BWD]->getUout());

    fast_switch->setHoldingCoilState(getHoldingCoilState());
    bv_return = getHoldingCoilState() && bv_return;
    fast_switch->setU_in(Ukr);
    fast_switch->setReturn(bv_return);
    fast_switch->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP2K::stepTracDrive(double t, double dt)
{
    tracForce = 0.0;

    for (size_t i = 0; i < motor.size(); ++i)
    {
        trac_conv[i]->setU_in(fast_switch->getU_out());
        trac_conv[i]->setRefCurrent(trac_reg->getRefAncorCurrent() + brake_reg->getRefAncorCurrent());
        trac_conv[i]->setCurrent(motor[i]->getAncorCurrent());

        motor[i]->setAncorVoltage(trac_conv[i]->getU_out());
        motor[i]->setFieldVoltage(field_conv->getU_out());
        motor[i]->setOmega(wheel_omega[i] * ip);
        motor[i]->setReversSate(1);

        Q_a[i+1] = motor[i]->getTorque() * ip;

        tracForce += Q_a[i+1] * 2.0 / wheel_diameter;

        trac_conv[i]->step(t, dt);
        motor[i]->step(t, dt);
    }

    field_conv->setU_in(fast_switch->getU_out() / 6.0);
    field_conv->setRefCurrent(trac_reg->getRefFieldCurrent() + brake_reg->getRefFieldCurrent());
    field_conv->setCurrent(motor[0]->getFieldCurrent());
    field_conv->step(t, dt);

    double trac_level = km->getPosition();

    trac_reg->setTractionLevel(trac_level);
    trac_reg->setOmega(wheel_omega[0]);
    trac_reg->setAncorCurrent(motor[0]->getAncorCurrent());
    trac_reg->step(t, dt);

    brake_reg->setBrakeLevel(trac_level);
    brake_reg->setAncorCurrent(motor[0]->getAncorCurrent());
    brake_reg->setOmega(wheel_omega[0]);
    brake_reg->step(t, dt);

    km->setControl(keys);
    km->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool EP2K::getHoldingCoilState()
{
    bool state = true;

    return state;
}
