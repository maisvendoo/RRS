//------------------------------------------------------------------------------
//
//      Магистральный пассажирский электровоз переменного тока ЧС4т.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Николай Авилкин (avilkin.nick)
//
//      Дата: 16/06/2019
//
//------------------------------------------------------------------------------
#ifndef KM21KR2_H
#define KM21KR2_H

#include "device.h"

struct ControlState
{
    double a2b2;
    double c2d2;
    double e2f2;
    double i2g2;
    double j2k2;
};

class Km21KR2 : public Device
{
public:
    Km21KR2(QObject *parent = Q_NULLPTR);

    double getA2B2();
    double getC2D2();
    double getE2F2();
    double getI2G2();
    double getJ2K2();

    void setUpState(bool state);
    void setUp1State(bool state);
    void setDown1State(bool state);
    void setDownState(bool state);

private:
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);
    void load_config(CfgReader &cfg);
    void preStep(state_vector_t &Y, double t);
    void stepKeysControl(double t, double dt);

    bool up;
    bool up1;
    bool zero;
    bool down1;
    bool down;

    ControlState ctrlState;
};

#endif // KM21KR2_H
