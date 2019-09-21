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

    void setHod(bool hod) { this->hod = hod; }

    ControllerState getCtrlState() { return controlState; }
    int getFieldStep() { return fieldStep ;}
    int getReverseState() { return reverseState; }

    double getMainShaftPos() { return mainShaftPos; }    

private:

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);
    void load_config(CfgReader &cfg);
    void preStep(state_vector_t &Y, double t);
    void stepKeysControl(double t, double dt);

    bool k21;
    bool k22;
    bool k23;

    bool k01;
    bool k02;

    bool n;
    bool p;
    bool re;
    bool hod;
    int fieldStep;
    int reverseState;

    double mainShaftPos;


    ControllerState controlState;
};

#endif // KM21KR2_H
