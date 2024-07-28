#ifndef     DC_MOTOR_H
#define     DC_MOTOR_H

#include    "device.h"

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

    /// Состояние звука работы
    virtual sound_state_t getSoundState(size_t idx = 0) const;

    /// Сигнал состояния звука работы
    virtual float getSoundSignal(size_t idx = 0) const;

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

    int     direction;

    double  torque_max;
    double  omega_nom;

    /// Состояние звука работы
    sound_state_t sound_state = sound_state_t(true, 0.0f, 0.0f);

    LinearInterpolation cPhi;

    QMap<int, double> fieldStep;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    double calcCPhi(double I);
};

#endif // DC_MOTOR_H
