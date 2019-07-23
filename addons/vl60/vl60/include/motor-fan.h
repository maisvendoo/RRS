#ifndef     MOTOR_FAN_H
#define     MOTOR_FAN_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MotorFan : public Device
{
public:

    MotorFan(size_t idx = 1, QObject *parent = Q_NULLPTR);

    ~MotorFan();

    void setU_power(double value);

    float isNoReady();

private:

    /// Индекс (по сути номер мотор-вентилятора в схеме)
    size_t  idx;

    double  Mmax;

    double  s_kr;

    double  Un;

    double  U_power;

    double  omega0;

    double  omega_nom;

    double  kr;

    double  J;

    double  is_no_ready;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // MOTOR_FAN_H
