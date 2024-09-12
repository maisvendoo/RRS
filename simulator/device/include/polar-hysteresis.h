#ifndef     POLAR_HYSTERESIS_H
#define     POLAR_HYSTERESIS_H

#include    <QObject>
#include    <device-export.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT PolarHysteresis : QObject
{
public:

    PolarHysteresis(double min_value = 0.1,
                    double max_value = 0.9,
                    int init_state = 0,
                    QObject *parent = Q_NULLPTR);

    ~PolarHysteresis();

    void setRange(double min_value, double max_value);

    void setValue(double value);

    int getState() const
    {
        return state;
    }

protected:

    double min = 0.1;

    double max = 0.9;

    int state = 0;
};

#endif
