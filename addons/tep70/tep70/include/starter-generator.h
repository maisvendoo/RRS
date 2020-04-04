#ifndef     STARTER_GENERATOR_H
#define     STARTER_GENERATOR_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class StarterGenerator : public Device
{
public:

    StarterGenerator(QObject *parent = Q_NULLPTR);

    ~StarterGenerator();

private:

    /// Напряжение на якоре
    double  Ua;

    /// Напряжение на обмотке возбуждения (в режиме генератора)
    double  Uf;

    /// Ток якоря
    double  Ia;

    /// Ток возбуждения
    double  If;

    /// Сопротивление обмотки якоря
    double  Ra;

    /// Сопротивление обмотки возбуждения
    double  Rf;

    /// Постоянная времени нарастания тока якоря
    double  Ta;

    /// Постоянная времени нарастания тока возбуждения
    double  Tf;

    /// Признак двигательного режима
    bool    is_motor;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void ode_system_motor(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void ode_system_generator(const state_vector_t &Y, state_vector_t &dYdt, double t);
};

#endif // STARTER_GENERATOR_H
