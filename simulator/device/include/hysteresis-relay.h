#ifndef     HYSTERESIS_RELAY_H
#define     HYSTERESIS_RELAY_H

#include    "hysteresis.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT HysteresisRelay : public Hysteresis
{
public:

    HysteresisRelay(double min_value = 0.1,
                    double max_value = 0.9,
                    bool init_state = false,
                    bool is_active = true,
                    bool is_locked = false,
                    QObject *parent = Q_NULLPTR);

    virtual ~HysteresisRelay();

    void setActive(bool is_active);

    void setLocked(bool is_locked);

    virtual void setValue(double value);

    virtual bool getState() const;

protected:

    bool    is_active;

    bool    is_locked;
};

#endif // HYSTERESIS_RELAY_H
