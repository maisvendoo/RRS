#ifndef     LOGIC_RELAY_H
#define     LOGIC_RELAY_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT LogicRelay : public Device
{
public:

    LogicRelay(size_t num_contacts = 1, QObject *parent = Q_NULLPTR);

    ~LogicRelay();

    void setInitContactState(size_t number, bool state);

    bool getContactState(size_t number) const;

    void setState(bool state);

signals:

    void changeState();

protected:

    bool state;

    bool state_old;

    std::vector<bool> contact;

    virtual void preStep(state_vector_t &Y, double t);

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);


    virtual void load_config(CfgReader &cfg);

    void change_contact_state();
};

#endif // LOGIC_RELAY_H
