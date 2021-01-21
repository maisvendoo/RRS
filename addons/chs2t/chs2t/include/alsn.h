#ifndef     ALSN_H
#define     ALSN_H

#include    "device.h"
#include    "alsn-struct.h"

class ALSN : public Device
{
public:

    ALSN(QObject *parent = Q_NULLPTR);

    ~ALSN();

    bool getSafetyState() const
    {
        return safety_relay.getState();
    }

    void setEPKstate(bool state_epk)
    {
        if (state_epk != old_stateEPK)
            safety_relay.reset();

        old_stateEPK = state_epk;
    }

    void setAlsnCode(short code_alsn)
    {
        if (code_alsn != old_code_alsn)
            safety_relay.reset();

        old_code_alsn = code_alsn;
    }

private:

    bool stateRB1;

    bool old_stateEPK;

    short old_code_alsn;

    Trigger safety_relay;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void preStep(state_vector_t &Y, double t);

    void stepExternalControl(double t, double dt);
};

#endif // ALSN_H
