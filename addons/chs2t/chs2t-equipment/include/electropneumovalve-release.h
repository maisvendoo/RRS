#ifndef     ELECTROPNEUMOVALVE_RELEASE_H
#define     ELECTROPNEUMOVALVE_RELEASE_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ElectroPneumoValveRelease : public Device
{
public:

    ElectroPneumoValveRelease(QObject *parent = Q_NULLPTR);

    ~ElectroPneumoValveRelease();

    /// Задать ток реостатного торможения
    void setEDTcurrent(double value);

    /// Отпуск пневматических тормозов при реостатном торможении
    bool isPneumoBrakesRelease() const;

    void step(double t, double dt);

private:

    double I;
    double Ia;

    bool release_valve_state;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // ELECTROPNEUMOVALVE_RELEASE_H
