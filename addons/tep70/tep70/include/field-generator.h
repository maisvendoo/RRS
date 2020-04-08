#ifndef     FIELD_GENERATOR_H
#define     FIELD_GENERATOR_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class FieldGenerator : public Device
{
public:

    FieldGenerator(QObject *parent = Q_NULLPTR);

    ~FieldGenerator();

    double getVoltage() const { return U; }

    double getTorque() const { return M; }

    void setFieldVoltage(double Uf) { this->Uf = Uf; }

    void setOmega(double omega) { this->omega = omega; }

    void setLoadCurrent(double In) { this->In = In; }

private:

    /// Угловая скорость вращения вала
    double  omega;

    /// Ток нагрузки
    double  In;

    /// Сопротивление ротора
    double  Ra;

    /// Сопротивление обмотки возбудения
    double  Rf;

    /// Напряжение на обмотке возбуждения
    double  Uf;

    /// Вырабатываемое напряжение
    double  U;

    /// Момент сопротивления на валу
    double  M;

    /// Коэффициент пропорциональности для расчета ЭДС
    double  Kf;

    /// Постоянная времени нарастания тока в ОВ
    double  Tf;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // FIELD_GENERATOR_H
