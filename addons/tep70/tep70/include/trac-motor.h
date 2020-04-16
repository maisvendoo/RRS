#ifndef     TRAC_MOTOR_H
#define     TRAC_MOTOR_H

#include    "device.h"

#include    "motor-magnetic-char.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TractionMotor : public Device
{
public:

    TractionMotor(QObject *parent = Q_NULLPTR);

    ~TractionMotor();

    void setAncorVoltage(double Ua) { this->Ua = Ua; }

    void setFieldVoltage(double Uf) { this->Uf = Uf; }

    double getAncorCurrent() const { return getY(0); }

    double getFieldCurrent() const  { return getY(1); }

    double getTorque() const { return M; }

    void setMode(bool is_motor) { this->is_motor = is_motor; }

    void setOmega(double omega) { this->omega = omega; }

    void setLoadCurrent(double In) { this->In = In; }

    void load_magnetic_char(QString file_name);

    void load_eff_coeff(QString file_name);

    double getEMF() const { return E; }

    void setFieldWeak(double beta) { this->beta = beta; }

    void setReversSate(int revers_state) { this->revers_state = revers_state; }

private:

    /// Напряжение якоря
    double  Ua;

    /// Напряжение на обмотке возбуждения (в режиме генератора)
    double  Uf;

    /// Сопротивление обмотки якоря (ОЯ)
    double  Ra;

    /// Постоянная времени нарастания тока в ОЯ
    double  Ta;

    /// Сопротивление обмотки возбуждения (ОВ)
    double  Rf;

    /// Сопротивление дополнительных полюсов ОВ
    double  Rd;

    /// Постоянная времени нарастания тока в ОВ
    double  Tf;

    /// Степень ослабления возбуждения
    double  beta;

    /// Состояние реверсора (1 - вперед, 0 - нейтрально, -1 - назад)
    int     revers_state;

    /// Момент на валу
    double  M;

    /// Признак двигательного режима
    bool    is_motor;

    /// Угловая скорость вращения вала
    double  omega;

    /// Ток нагрузки в генератором режиме
    double  In;

    double  E;

    /// Характеристика намагничивания
    MotorMagneticChar   magnetic_char;

    MotorMagneticChar   eff_coef;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    double cPhi(double If);
};

#endif // TRACMOTOR_H
