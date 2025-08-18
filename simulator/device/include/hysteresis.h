#ifndef     HYSTERESIS_H
#define     HYSTERESIS_H

#include    "device-export.h"
#include    "trigger.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT Hysteresis : public Trigger
{
public:

    Hysteresis(double min_value = 0.1,
               double max_value = 0.9,
               bool init_state = false);

    ~Hysteresis() = default;

    void setRange(double min_value, double max_value);

    virtual void setValue(double value);

protected:

    double min = 0.1;
    double max = 0.9;

    void set() { Trigger::set(); }
    void reset() { Trigger::reset(); }
};

#endif // HYSTERESIS_H
