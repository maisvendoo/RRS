#include    "ep1m.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double EP1m::calcTracForce()
{
    double trac_force = 0;

    for (size_t i = 1; i < Q_a.size(); ++i)
    {
        trac_force += 2 * Q_a[i] / wheel_diameter[i - 1];
    }

    return trac_force;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::stepRegistration(const double& t, const double& dt)
{
    QString line = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9")
            .arg(t, 10, 'f', 1) // Текущее время симуляции
            .arg(velocity * Physics::kmh, 6, '2', 1) // Текущая скорость
            .arg(trac_motor[0]->getAncorCurrent(), 7, 'f', 2) // Ток якоря
            .arg(trac_motor[0]->getFieldCurrent(), 7, 'f', 2) // Ток возбуждения
            .arg(calcTracForce() / 1000.0, 6, '2', 1) // Сила тяги/ЭДТ
            .arg(main_reservoir->getPressure(), 7, 'f', 3) // Давление ГР
            .arg(brakepipe->getPressure(), 7, 'f', 3) // Давление ТМ
            .arg(supply_reservoir->getPressure(), 7, 'f', 3) // Давление ЗР
            .arg(brake_mech[TROLLEY_FWD]->getBCpressure(), 7, 'f', 3); // Давление ТЦ

    registrator->print(line, t, dt);
}
