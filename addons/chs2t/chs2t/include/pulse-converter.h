#ifndef PULSECONVERTER_H
#define PULSECONVERTER_H

#include "device.h"

class DCPulseConverter : public Device
{

public:

    ///Конструктор
    DCPulseConverter(QObject *parent = Q_NULLPTR);

    ///Деструктор
    ~DCPulseConverter();

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
