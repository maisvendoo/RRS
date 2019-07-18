#ifndef     TRAC_TRANSFORMER_H
#define     TRAC_TRANSFORMER_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TracTransformer : public Device
{
public:

    TracTransformer(QObject *parent = Q_NULLPTR);

    ~TracTransformer();

    /// Получить напряжение с обмотки собственных нужд (СН)
    double getU_sn() const;

    /// Задать напряжение первичной обмотки
    void setU1(double value);

private:

    /// Напряжение на первичной обмотке
    double  U1;    

    /// Коэффициент трансформации обмотки СН
    double  K_sn;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // TRAC_TRANSFORMER_H
