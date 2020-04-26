#ifndef SWITCHER_H
#define SWITCHER_H

#include "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT Switcher : public Device
{
public:

    Switcher(QObject *parent = Q_NULLPTR, int key_code = 0, int kol_states = 0);

    ~Switcher();

    void setKeyCode(int value) { keyCode = value; }

    void setKolStates(int value) { kolStates = value; is_switched.resize(static_cast<size_t>(kolStates)); }

    void setState(int value) { state = value; }

    int getState() const { return state; }

    int getKolStates() const { return kolStates; }

    float getHandlePos() const { return static_cast<float>(state) / static_cast<float>(kolStates - 1) ; }

    bool isSwitched(int pos) const
    {
        if (pos < static_cast<int>(is_switched.size()))
            return is_switched[static_cast<size_t>(state)];
        else
            return false;
    }

    void setPlusSoundName(QString soundName);

    void setMinusSoundName(QString soundName);

protected:

    int keyCode;

    int state;

    int kolStates;

    bool ableToPress;

    std::vector<bool> is_switched;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void stepKeysControl(double t, double dt);

    QString PlusSoundName;

    QString MinusSoundName;
};

#endif // SWITCHER_H
