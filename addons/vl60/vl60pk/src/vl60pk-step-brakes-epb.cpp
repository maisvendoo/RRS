#include    "vl60pk.h"

#include "brake-lock.h"
#include "brake-crane.h"
#include "electro-airdistributor.h"
#include "epb-2line-control.h"
#include "epb-converter.h"
#include "pneumo-hose-epb.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60pk::stepEPB(double t, double dt)
{
    // Потребляемый ток в рабочей линии ЭПТ
    double evr_current = electro_air_dist->getCurrent(0);

    // Потребляемый ток в рабочей линии ЭПТ
    double epb_work_curr = 0.0;
    epb_work_curr += evr_current;
    epb_work_curr += hose_bp_fwd->getCurrent(0);
    epb_work_curr += hose_bp_bwd->getCurrent(0);

    // Преобразователь напряжения для ЭПТ
    epb_converter->setInputVoltage(U_bat);
    epb_converter->setOutputCurrent(epb_work_curr);
    epb_converter->step(t, dt);

    // Контроллер двухпроводного ЭПТ
    bool cab1_on = brake_lock[CAB1]->isUnlocked() && epb_switch[CAB1].getState();
    bool cab2_on = brake_lock[CAB2]->isUnlocked() && epb_switch[CAB2].getState();
    epb_control->setInputVoltage(epb_converter->getOutputVoltage() *
                                 static_cast<double>(cab1_on || cab2_on) );
    epb_control->setHoldState((cab1_on && brake_crane[CAB1]->isHold()) ||
                              (cab2_on && brake_crane[CAB2]->isHold()));
    epb_control->setBrakeState((cab1_on && brake_crane[CAB1]->isBrake()) ||
                               (cab2_on && brake_crane[CAB2]->isBrake()));
    epb_control->setControlVoltage(hose_bp_fwd->getVoltage(1) + hose_bp_bwd->getVoltage(1));
    epb_control->step(t, dt);
    double epb_work_U = epb_control->getWorkVoltage();
    double epb_work_f = epb_control->getWorkFrequency();

    // Управление электровоздухораспределителем
    electro_air_dist->setVoltage  (0,  epb_work_U
        + hose_bp_fwd->getVoltage(0) + hose_bp_bwd->getVoltage(0) );
    electro_air_dist->setFrequency(0,  epb_work_f
        + hose_bp_fwd->getFrequency(0) + hose_bp_bwd->getFrequency(0) );

    // Межвагонные сигналы линий ЭПТ по рукавам тормозной магистрали
    // Рабочая линия спереди
    hose_bp_fwd->setVoltage  (0, hose_bp_bwd->getVoltage(0) + epb_work_U);
    hose_bp_fwd->setFrequency(0, hose_bp_bwd->getFrequency(0) + epb_work_f);
    hose_bp_fwd->setCurrent  (0, hose_bp_bwd->getCurrent(0) + evr_current);
    // Контрольная линия спереди
    hose_bp_fwd->setVoltage  (1, hose_bp_bwd->getVoltage(1));
    hose_bp_fwd->setFrequency(1, hose_bp_bwd->getFrequency(1));
    hose_bp_fwd->setCurrent  (1, hose_bp_bwd->getCurrent(1));

    // Рабочая линия сзади
    hose_bp_bwd->setVoltage  (0, hose_bp_fwd->getVoltage(0) + epb_work_U);
    hose_bp_bwd->setFrequency(0, hose_bp_fwd->getFrequency(0) + epb_work_f);
    hose_bp_bwd->setCurrent  (0, hose_bp_fwd->getCurrent(0) + evr_current);
    // Контрольная линия сзади
    hose_bp_bwd->setVoltage  (1, hose_bp_fwd->getVoltage(1));
    hose_bp_bwd->setFrequency(1, hose_bp_fwd->getFrequency(1));
    hose_bp_bwd->setCurrent  (1, hose_bp_fwd->getCurrent(1));
}
