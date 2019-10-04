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

    double getStepR(int pos) {return steps[pos];}

private:

    int poz;

    double R;

    QMap<int, double> steps;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);
    void load_config(CfgReader &cfg);
    void preStep(state_vector_t &Y, double t);

};

#endif // PUSKREZ_H
