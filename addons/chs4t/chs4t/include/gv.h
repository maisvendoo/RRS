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
#ifndef GV_H
#define GV_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class GV : public Device
{
public:

    GV(QObject *parent = Q_NULLPTR);



    double getX();
    double getUout();

    void setP0(double _P0);
    void setP1(double _P1);

    void setUkr(double Ukr);

    void setGVState(bool state);
    void setVZState(bool state);

    void setPhc(bool phc);

private:
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);
    void load_config(CfgReader &cfg);
    void preStep(state_vector_t &Y, double t);
    void stepKeysControl(double t, double dt);



    CfgReader cfg_;


    double  Uout;
    double  Ukr;

    double Vn;
    double Vdk;

    double Fk;
    double Fp;

    double P0;
    double P1;

    double K1;
    double K2;

    double sdk;
    double Pdk;

    bool GVstate;
    bool VZstate;
    double tVZ;
    ///
    bool phc;

};

#endif // GV_H
