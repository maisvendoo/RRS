#ifndef PUSKREZ_H
#define PUSKREZ_H

#include "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class PuskRez : public Device
{
public:

    ///Конструктор
    PuskRez(QObject *parent = Q_NULLPTR);

    ///Деструктор
    ~PuskRez();

    void setPoz(int poz) { this->poz = poz; }

    double getR() { return R; }

private:
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);
    void load_config(CfgReader &cfg);
    void preStep(state_vector_t &Y, double t);
    void stepKeysControl(double t, double dt);

    QMap<int, double> steps;

    int poz;

    double R;

};

#endif // PUSKREZ_H
