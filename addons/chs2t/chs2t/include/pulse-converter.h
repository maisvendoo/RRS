#ifndef PULSECONVERTER_H
#define PULSECONVERTER_H

#include "device.h"

class PulseConverter : public Device
{

public:

    ///Конструктор
    PulseConverter(QObject *parent = Q_NULLPTR);

    ///Деструктор
    ~PulseConverter();

    void setU(double value) { u = value; }

    void setUakb(double value) { Uakb = value; }

    void setUt(double value) { Ut = value; }

    double getUf() const { return Uf; }

private:

    double u;
    double Uakb;
    double Ut;
    double Uf;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void preStep(state_vector_t &Y, double t);
};

#endif // PULSECONVERTER_H
