#ifndef     ELECTRIC_OIL_PUMP_H
#define     ELECTRIC_OIL_PUMP_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ElectricOilPump : public Device
{
public:

    ElectricOilPump(QObject *parent = Q_NULLPTR);

    ~ElectricOilPump();

    void setVoltage(double U) { this->U = U; }

    double getOilFlow() const;

    double getCurrent() const;

private:

    /// Напряжение питания, В
    double  U;

    /// Ток якоря, А
    double  Ia;

    /// Ток возбуждения, А
    double  If;

    /// Сопротивление обмотки якоря, Ом
    double  Ra;

    /// Сопротивление обмотки возбуждения
    double  Rf;

    /// Коэффициент противо-ЭДС при номинальном токе
    double  cF;

    /// Момент инерции подвижных частей насоса
    double  J;

    /// Номинальные обороты двигателя
    double omega_nom;

    /// Коэффициент сопротивления вращению насоса
    double  kc;

    /// Расход масла, подаваемого в рампу ТНВД
    double  Q_oil;

    /// Расход масла при номинальных оборотах
    double  Q_nom;

    /// Признак запуска
    bool    is_started;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // ELECTRICOILPUMP_H
