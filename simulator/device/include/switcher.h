#ifndef SWITCHER_H
#define SWITCHER_H

#include "device.h"

class DEVICE_EXPORT Switcher : public Device
{
public:
    Switcher(QObject *parent = Q_NULLPTR);

    ~Switcher();

    void setKeyCode(int value) { keyCode = value; }

protected:
    int keyCode;
    int state;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

};

#endif // SWITCHER_H
