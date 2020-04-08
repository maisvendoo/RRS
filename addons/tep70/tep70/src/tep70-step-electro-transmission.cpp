#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::stepElectroTransmission(double t, double dt)
{
    // Состояние цепи обмотки возбуждения возбудителя
    bool is_FGF_on = kontaktor_starter->getContactState(1);

    field_gen->setFieldVoltage(Ucc * static_cast<double>(is_FGF_on));
    field_gen->setLoadCurrent(0.0);
    field_gen->setOmega(disel->getStarterOmega());
    field_gen->step(t, dt);

    // Ток, потребляемый от главного генератора
    I_gen = 0.0;

    trac_gen->setLoadCurrent(I_gen);
    trac_gen->step(t, dt);

    field_reg->step(t, dt);
}
