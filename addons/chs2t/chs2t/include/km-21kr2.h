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
#include "km-21kr2-state.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Km21KR2 : public Device
{
public:
    /// Конструктор
    Km21KR2(QObject *parent = Q_NULLPTR);

    ///Деструктор
    ~Km21KR2();

    ControllerState getCtrlState() { return controlState; }

private:
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);
    void load_config(CfgReader &cfg);
    void preStep(state_vector_t &Y, double t);
    void stepKeysControl(double t, double dt);

    bool k21;
    bool k22;
    bool k23;
    int n;

    ControllerState controlState;
};

#endif // KM21KR2_H
