#include    "chs2t.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void CHS2T::stepPneumoSupply(double t, double dt)
{
    // Регулятор давления
    press_reg->setFLpressure(main_reservoir->getPressure());
    press_reg->step(t, dt);

    double FL_flow = 0.0;

    // Мотор-компрессоры
    for (size_t i = 0; i < motor_compressor.size(); ++i)
    {
        mk_switcher[i]->setControl(keys);
        mk_switcher[i]->step(t, dt);

        double U_power = 0.0;

        if (    (mk_switcher[i]->getState() == 3)
            || ((mk_switcher[i]->getState() == 2) && (press_reg->getState() )) )
        {
            U_power = bv->getU_out();
        }
        motor_compressor[i]->setFLpressure(main_reservoir->getPressure());
        motor_compressor[i]->setPowerVoltage(U_power);
        motor_compressor[i]->step(t, dt);

        FL_flow += motor_compressor[i]->getFLflow();
    }

    // Главные резервуары
    FL_flow += brake_crane->getFLflow();
    FL_flow += loco_crane->getFLflow();
    FL_flow += epk->getFLflow();
    FL_flow += dako->getFLflow();
    FL_flow += bc_pressure_relay->getFLflow();
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
