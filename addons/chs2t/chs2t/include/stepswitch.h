#ifndef     STEPSWITCH_H
#define     STEPSWITCH_H

#include    "device.h"
#include    "km-21kr2-state.h"

#include   "ampermeters-state.h"

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

    void setDropPosition(bool value) { dropPosition = value; }

    int getPoz() { return poz; }

    double getSchemeState() const;

    bool getHod() { return hod; }

    int getFieldStep() const { return fieldStep; }

    bool isZero() const;

    bool isSeries() const;

    bool isSeriesParallel() const;

    bool isParallel() const;

    ampermeters_state_t getAmpermetersState();



private:

    enum
    {
        MPOS_S = 20,
        MPOS_SP = 33,
        MPOS_P = 42
    };

    double V;

    double poz_d;

    int poz;

    int fieldStep;

    bool n;
    bool p;
    bool s;
    int prevPos;

    bool dropPosition;

    bool hod;

    ControllerState ctrlState;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void preStep(state_vector_t &Y, double t);

    void stepDiscrete(double t, double dt);
};


#endif // STEPSWITCH_H
