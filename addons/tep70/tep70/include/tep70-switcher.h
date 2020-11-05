#ifndef     TEP70_SWITCHER_H
#define     TEP70_SWITCHER_H

#include    "device.h"
#include    "switcher.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TEP70Switcher : public Switcher
{
public:

    TEP70Switcher(QObject *parent = Q_NULLPTR, int key_code = 0, int kol_states = 0);

    ~TEP70Switcher();

    void setSoundName(QString soundName) { this->soundName = soundName; }

protected:

    int     old_state;

    QString soundName;

    void preStep(state_vector_t &Y, double t);
};

#endif // TEP70_SWITCHER_H
