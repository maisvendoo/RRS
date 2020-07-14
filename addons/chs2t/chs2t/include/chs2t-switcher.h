#ifndef     CHS2T_SWITCHER_H
#define     CHS2T_SWITCHER_H

#include    "device.h"
#include    "switcher.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class CHS2TSwitcher : public Switcher
{
public:

    CHS2TSwitcher(QObject *parent = Q_NULLPTR, int key_code = 0, int kol_states = 0);

    ~CHS2TSwitcher();

    void setSoundName(QString soundName) { this->soundName = soundName; }

    void setSpring(int state, int spring_state) { springStates.insert(state, spring_state); }

protected:

    int     old_state;

    QString soundName;

    QMap<int, int> springStates;

    void preStep(state_vector_t &Y, double t);
};

#endif // CHS2T_SWITCHER_H
