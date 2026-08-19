#ifndef     SPOT_LIGHT_H
#define     SPOT_LIGHT_H

#include    <device.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT SpotLight : public Device
{
public:

    SpotLight(QObject *parent = Q_NULLPTR);

    ~SpotLight();

    void setState(bool is_low, bool is_high)
    {
        this->is_low = is_low;
        this->is_high = is_high;
    }

    double getIntensityHigh() const
    {
        return intensity_high;
    }

    double getIntensityLow() const
    {
        return intensity_low;
    }

private:

    /// Уровень интенсивности свечения "Ярко"
    double intensity_high = 0.0;

    /// Уровень интенсивности свечения "Ярко"
    double intensity_low = 0.0;

    /// Максимальный уровень интенсивности "Тускло"
    double low_level = 0.5;

    bool is_low = false;

    bool is_high = false;

    virtual void preStep(state_vector_t &Y, double t);

    virtual void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    virtual void load_config(CfgReader &cfg);
};

#endif // SPOT_LIGHT_H
