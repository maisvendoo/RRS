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

    int getReverseState() const { return reverseState; }

    double getMainShaftPos() const { return mainShaftPos * 0.1; }

    double getMainShaftHeight() const { return mainShaftHeight; }

private:

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void preStep(state_vector_t &Y, double t);

    void stepKeysControl(double t, double dt);

    bool k21;
    bool k22;
    bool k23;

    bool k31;
    bool k32;
    bool k33;

    bool k01;
    bool k02;

    bool autoSet;
    bool autoReset;
    bool p;
    bool isPressedOneTime;
    bool hod;

    int reverseState;

    int mainShaftPos;
    int fieldWeakShaft;

    double mainShaftHeight;

    ControllerState controlState;
};

#endif // KM21KR2_H
