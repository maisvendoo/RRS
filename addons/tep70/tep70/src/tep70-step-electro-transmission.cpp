#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::stepElectroTransmission(double t, double dt)
{
    // Состояние цепи контактора возбуждения возбудителя
    bool is_KVV_on = azv_upr_tepl.getState() && km->isNoZero();

    kvv->setVoltage(Ucc * static_cast<double>(is_KVV_on));
    kvv->step(t, dt);

    // Состояние цепи контактора возбуждения генератора
    bool is_KVG_on = azv_upr_tepl.getState() && km->isNoZero();

    kvg->setVoltage(Ucc * static_cast<double>(is_KVG_on));
    kvg->step(t, dt);

    // Состояние цепи обмотки возбуждения возбудителя
    bool is_FGF_on = kontaktor_starter->getContactState(1) ||
                     kvv->getContactState(0);

    field_gen->setFieldVoltage(Ucc * static_cast<double>(is_FGF_on));
    field_gen->setLoadCurrent(trac_gen->getFieldCurrent());
    field_gen->setOmega(disel->getStarterOmega());
    field_gen->step(t, dt);

    // Ток, потребляемый от главного генератора
    I_gen = trac_gen->getVoltage() / 0.01;

    // Состояние цепи возбуждения главного генератора
    bool is_TGF_on = kvg->getContactState(0);

    trac_gen->setFieldVoltage(field_reg->getFieldVoltage() * static_cast<double>(is_TGF_on));
    trac_gen->setLoadCurrent(I_gen);
    trac_gen->setOmega(disel->getOmega());
    trac_gen->step(t, dt);

    field_reg->setFGVoltage(field_gen->getVoltage());
    field_reg->setOmega(disel->getOmega());
    field_reg->setGenVoltage(trac_gen->getVoltage());
    field_reg->setGenCurrent(I_gen);
    field_reg->setKMPosition(km->getPositionNumber());
    field_reg->step(t, dt);
}
