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

    void setIa(double value) { Ia = value; }

    void setIf(double value) { If = value; }

    void setBref(double value) { Bref = value; }

    double getU() const { return u; }

    void setAllowEDT(bool value) { allowEDT = value; }

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

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void preStep(state_vector_t &Y, double t);
};

#endif // BRAKEREGULATOR_H
