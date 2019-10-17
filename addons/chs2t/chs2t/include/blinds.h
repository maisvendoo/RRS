#ifndef     BLINDS_H
#define     BLINDS_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Blinds : public Device
{
public:

    Blinds(QObject *parent = Q_NULLPTR);

    ~Blinds();

    void setState(bool state);

    bool isOpened() const;

    float getPosition() const;

private:

    double  motion_time;

    bool    state;

    bool    is_opened;

    double  max_pos;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // BLINDS_H
