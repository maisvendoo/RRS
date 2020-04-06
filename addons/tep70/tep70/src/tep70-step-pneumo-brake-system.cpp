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
}
