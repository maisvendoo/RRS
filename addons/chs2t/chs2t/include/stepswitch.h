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

    int getPoz() { return poz; }

private:
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);
    void load_config(CfgReader &cfg);
    void preStep(state_vector_t &Y, double t);
    void stepKeysControl(double t, double dt);


    ControllerState ctrlState;

    double V;

    double poz_d;

    int poz;

    int n;

};


#endif // STEPSWITCH_H
