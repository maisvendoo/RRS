#ifndef     DISEL_H
#define     DISEL_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Disel : public Device
{
public:

    Disel(QObject *parent = Q_NULLPTR);

    ~Disel();

    /// Задать расход масла от электрического маслопрокачивающего насоса (ЭМН)
    void setQ_emn(double Q_emn);

    double getOilPressure() const { getY(0); }

private:

    enum
    {
        NUM_COEFFS = 10,
        OIL_PRESSURE = 0,
        SHAFT_OMEGA = 1,
    };

    /// Условный объем маслянного русла
    double V_oil;

    /// Расход масла от ЭМН
    double Q_emn;

    std::array<double, NUM_COEFFS>  K;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // DISEL_H
