//------------------------------------------------------------------------------
//
//      Магистральный пассажирский электровоз постоянного тока ЧС2т.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Николай Авилкин (avilkin.nick)
//
//      Дата: 21/08/2019
//
//------------------------------------------------------------------------------
#ifndef PANTOGRAPH_H
#define PANTOGRAPH_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Pantograph : public Device
{
public:

    ///Конструктор
    Pantograph(QObject *parent = Q_NULLPTR);

    ///Деструктор
    ~Pantograph();

    void setUks(double Uks)   { this->Uks = Uks; }
    void setState(bool state) { this->state = state; }

    double getH()    { return h; }
    double getUout() { return Uout; }


private:
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);
    void load_config(CfgReader &cfg);
    void preStep(state_vector_t &Y, double t);
    void stepKeysControl(double t, double dt);


    double  tau;

    double  h;
    double hMax;

    double  V;

    double  Uout;
    double  Uks;

    bool state;

};

#endif // PANTOGRAPH_H
