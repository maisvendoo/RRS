#ifndef     DC_MOTOR_H
#define     DC_MOTOR_H

#include    "device.h"
#include    "motor-magnetic-char.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DCMotor : public Device
{
public:

    DCMotor(QObject *parent = Q_NULLPTR);

    ~DCMotor();

    void setOmega(double value);

    void setU(double value);

    double getTorque() const;

    double getIa() const;

    double getIf() const;

    double getUd() const;

    void setBeta(double value);

    void setBetaStep(int step);

    void setDirection(int revers_state);

private:

    /// Степень ослабления возбуждения
    double  beta;

    /// Сопротивление обмотки якоря (ОЯ)
    double  R_a;

    /// Сопротивление главных полюсов ОВ
    double  R_gp;

    /// Сопротивление добавочных полюсов
    double  R_dp;

    double  R_r;

    /// Эквивадентная индуктивность обмоток
    double  L_af;

    /// Угловая скорость вала
    double  omega;

    /// Напряжение
    double  U;

    /// Момент на валу
    double torque;

    double  omega_nom;

    int     direction;    

    MotorMagneticChar cPhi;

    QMap<int, double> fieldStep;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    double calcCPhi(double I);
};

#endif // DC_MOTOR_H
