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
#ifndef STEPSWITCH_H
#define STEPSWITCH_H

#include "device.h"
#include "km-21kr2-state.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class StepSwitch : public Device
{
public:

    ///Конструктор
    StepSwitch(QObject *parent = Q_NULLPTR);

    ///Деструктор
    ~StepSwitch();

    void setCtrlState(ControllerState ctrlState) { this->ctrlState = ctrlState; }

    double getv1() { return  v1; }
    double getv2() { return  v2; }
    int getPoz() { return poz; }
    double getSr() { return shaft_rel; }

private:
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);
    void load_config(CfgReader &cfg);
    void preStep(state_vector_t &Y, double t);
    void stepKeysControl(double t, double dt);


    ControllerState ctrlState;

    Trigger *rs;

//    int steps;

//    double f1 -- Y[0];

    double s62;
    double s67;
    double s69;
    double s45;
    double s43;
    double s410;

    double shaft_rel;

    double PI;
    double p1;
    double p2;
    double p3;
    double p4;

    double KL;
    double AB;
    double GH;
    double CD;
    double EF;
    double IJ;
    double NM;
    double UV;
    double HG;

    double v1;
    double v2;
    double u;

    double ang_vel;
    int poz;

};


#endif // STEPSWITCH_H
