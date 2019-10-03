#ifndef BRAKEREGULATOR_H
#define BRAKEREGULATOR_H

#include "device.h"

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
    double Ia;
    double If;
    double Bref;
    double u;

    double k1;
    double k2;

    bool allowEDT;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void preStep(state_vector_t &Y, double t);
};

#endif // BRAKEREGULATOR_H
