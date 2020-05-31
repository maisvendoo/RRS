#ifndef     TRACTION_REGULATOR_H
#define     TRACTION_REGULATOR_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TractionRegulator : public Device
{
public:

    TractionRegulator(QObject *parent = Q_NULLPTR);

    ~TractionRegulator();

    void setTractionLevel(double trac_level)
    {
        this->trac_level = cut(trac_level, 0.0, 1.0);
    }

    void setAncorCurrent(double Ia) { this->Ia = Ia; }

    void setOmega(double omega) { this->omega = omega; }

    double getRefAncorCurrent() const { return Ia_ref; }

    double getRefFieldCurrent() const { return If_ref; }

private:

    /// Максимальная уставка по току якоря и току возбуждения
    double I_max;

    /// Угловая скорость КП перехода на киперболическую часть характеристики
    double omega_nom;

    /// Уровень тягового усилия, заданный с КМ
    double trac_level;

    /// Текущий ток якоря ТЭД
    double Ia;

    /// Текущая угловая скорость КП
    double omega;

    /// Заданный ток якоря ТЭД
    double Ia_ref;

    /// Заданный ток возбуждения ТЭД
    double If_ref;

    /// Коэффициент обратной связи по току якоря
    double K;

    /// Минимальная степерь ослабление возбуждения
    double beta_min;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &, state_vector_t &, double);

    void load_config(CfgReader &cfg);
};

#endif // TRACTION_REGULATOR_H
