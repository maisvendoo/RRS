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

    double getSchemeState() const;

    bool getHod() { return hod; }

private:
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);
    void load_config(CfgReader &cfg);
    void preStep(state_vector_t &Y, double t);
    void stepKeysControl(double t, double dt);


    enum
    {
        MPOS_S = 20,
        MPOS_SP = 33,
        MPOS_P = 42
    };

    ControllerState ctrlState;

    double V;
    double V1;


    double poz_d;

    int poz;

    int n;
    int p;

    bool hod;

};


#endif // STEPSWITCH_H
