#ifndef     HYSTERESIS_POLAR_H
#define     HYSTERESIS_POLAR_H

#include    <QObject>

#include    <device-export.h>
#include    "sound-signal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT HysteresisPolar
{
public:

    HysteresisPolar(double min_value = 0.1,
                    double max_value = 0.9,
                    int init_state = 0);

    ~HysteresisPolar() = default;

    /// Задать начальное состояние (без озвучки)
    void setInitState(int init_state);

    void setRange(double min_value, double max_value);

    void setValue(double value);

    int getState() const;

    enum {
        CHANGE_SOUND = 0,   ///< Звук переключения
    };
    /// Состояние звука
    virtual sound_state_t getSoundState(size_t idx = CHANGE_SOUND) const;

    /// Сигнал состояния звука
    virtual float getSoundSignal(size_t idx = CHANGE_SOUND) const;

protected:

    double min = 0.1;

    double max = 0.9;

    int state = 0;

    sound_state_t sound_change_state = sound_state_t();
};

#endif // HYSTERESIS_POLAR_H
