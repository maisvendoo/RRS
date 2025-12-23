#ifndef     BRAKE_SHOES_H
#define     BRAKE_SHOES_H

#include    <device.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT BrakeShoes : public Device
{
public:

    BrakeShoes(QObject *parent = Q_NULLPTR);

    ~BrakeShoes();

    void setAxisLoad(double N_axis)
    {
        this->N_axis = N_axis;
    }

    double getForce() const
    {
        return brake_force;
    }

    void set()
    {
        state = true;
    }

    void reset()
    {
        state = false;
    }

protected:

    /// Коэффициент трения между башмаками и рельсом
    double fric_coeff = 0.25;

    /// Сила прижатия башмака к рельсу (осевая нагрузка)
    double N_axis = 0.0;

    /// Тормозная сила
    double brake_force = 0.0;

    /// Состояние башмака (подложен/не подложен)
    bool state = false;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) override;

    void preStep(state_vector_t &Y, double t) override;

    void load_config(CfgReader &cfg) override;
};

#endif
