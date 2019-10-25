#ifndef BRAKEREGULATOR_H
#define BRAKEREGULATOR_H

#include "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class BrakeRegulator : public Device
{
public:

    ///Конструктор
    BrakeRegulator(QObject *parent = Q_NULLPTR);

    ///Деструктор
    ~BrakeRegulator();

    void setIa(double value)     { Ia = value; }

    void setIf(double value)     { If = value; }

    void setBref(double value)   { Bref = value; }

    void setAllowEDT(bool value) { allowEDT = value; }


    double getU() const             { return u; }

    void reset() { setY(0, 0); setY(1, 0); }

private:

    /// Текущий ток якорей ТЭД
    double Ia;

    /// Текущий ток возбуждения
    double If;

    /// Заданное тормозное усилие
    double Bref;

    /// Управляющее воздействие для ИПВ (уровень напряжения на обмотке возбуждения)
    double u;

    /// Коэффициент пропорциональности между давлением в ЗТ и заданным током якоря
    double k_1;

    /// Передаточный коэффициент интегрального регулятора
    double k_2;

    /// Флаг разрешающий работу ЭДТ
    bool allowEDT;

    /// Постоянная времени нарастания заданного тока
    double T;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void preStep(state_vector_t &Y, double t);

    void load_config(CfgReader &cfg);
};

#endif // BRAKEREGULATOR_H
