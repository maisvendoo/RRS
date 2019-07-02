//------------------------------------------------------------------------------
//
//      Магистральный пассажирский электровоз переменного тока ЧС4т.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Авилкин Николай (avilkinnick)
//
//      Дата: 16/06/2019
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

    Pantograph(QObject *parent = Q_NULLPTR);

    double getH();

    double getUout();

    void setUks(double _Uks);

    void setState(bool state);

private:
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);
    void load_config(CfgReader &cfg);
    void preStep(state_vector_t &Y, double t);
    void stepKeysControl(double t, double dt);



    CfgReader cfg_;

    double  tau;

    double  h;
    double hMax;

//    double  upTime;
//    double  downTime;

    double  V;

    double  Uout;
    double  Uks;

    bool state;

//    enum PantOrder{PANT_FORWARD,
//                  PANT_BACKWARD};

};

#endif // PANTOGRAPH_H
