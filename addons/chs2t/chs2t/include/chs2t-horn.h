#ifndef     CHS2T_HORN_H
#define     CHS2T_HORN_H

#include    "train-horn.h"
#include    "hardware-signals.h"

class CHS2tHorn : public TrainHorn
{
public:

    CHS2tHorn(QObject *parent = Q_NULLPTR);

    ~CHS2tHorn();

/*private:

<<<<<<< HEAD
    void stepKeysControl(double t, double dt)
    {
        //TrainHorn::stepKeysControl(t, dt);

        //if (control_signals.analogSignal[KM_SVISTOK].is_active)
        //{
            if (is_svistok = static_cast<bool>(control_signals.analogSignal[KM_SVISTOK].cur_value))
            {
                emit soundSetVolume("Svistok", 100);
            }
            else
            {
                emit soundSetVolume("Svistok", 0);
            }
        //}
    }*/
    void stepKeysControl(double t, double dt);
};

#endif // CHS2T_HORN_H
