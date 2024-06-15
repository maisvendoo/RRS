#include    "passcar.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void PassCar::stepEPB(double t, double dt)
{
    (void) t;
    (void) dt;

    // Управление электровоздухораспределителем
    double evr_current = 0.0;
    if (electro_air_dist != nullptr)
    {
        electro_air_dist->setVoltage(0,
            hose_bp_fwd->getVoltage(0) + hose_bp_bwd->getVoltage(0) );
        electro_air_dist->setFrequency(0,
            hose_bp_fwd->getFrequency(0) + hose_bp_bwd->getFrequency(0) );
        evr_current = electro_air_dist->getCurrent(0);
    }

    // Межвагонные сигналы линий ЭПТ по рукавам тормозной магистрали
    // Рабочая линия спереди
    hose_bp_fwd->setVoltage  (0, hose_bp_bwd->getVoltage(0));
    hose_bp_fwd->setFrequency(0, hose_bp_bwd->getFrequency(0));
    hose_bp_fwd->setCurrent  (0, hose_bp_bwd->getCurrent(0) + evr_current);
    // Контрольная линия спереди
    hose_bp_fwd->setVoltage  (1, hose_bp_bwd->getVoltage(1));
    hose_bp_fwd->setFrequency(1, hose_bp_bwd->getFrequency(1));
    hose_bp_fwd->setCurrent  (1, hose_bp_bwd->getCurrent(1));

    // Рабочая линия сзади
    hose_bp_bwd->setVoltage  (0, hose_bp_fwd->getVoltage(0));
    hose_bp_bwd->setFrequency(0, hose_bp_fwd->getFrequency(0));
    hose_bp_bwd->setCurrent  (0, hose_bp_fwd->getCurrent(0) + evr_current);
    // Контрольная линия сзади
    hose_bp_bwd->setVoltage  (1, hose_bp_fwd->getVoltage(1));
    hose_bp_bwd->setFrequency(1, hose_bp_fwd->getFrequency(1));
    hose_bp_bwd->setCurrent  (1, hose_bp_fwd->getCurrent(1));
}
