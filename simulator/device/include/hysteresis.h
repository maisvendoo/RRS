#ifndef     HYSTERESIS_H
#define     HYSTERESIS_H

#include    <QObject>

#include    "device-export.h"
#include    "trigger.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT Hysteresis : public QObject
{
public:

    Hysteresis(double min_value = 0.1,
               double max_value = 0.9,
               bool init_state = false,
               QObject *parent = Q_NULLPTR);

    ~Hysteresis();

    void setRange(double min_value, double max_value);

    virtual void setValue(double value);

    virtual bool getState() const;

    sound_state_t getSoundOn() const;

    sound_state_t getSoundOff() const;

protected:

    double min;

    double max;

    Trigger state;
};

#endif // HYSTERESIS_H
