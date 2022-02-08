#ifndef     KPD3_H
#define     KPD3_H

#include    "device.h"

class KPD3 : public Device
{
public:

    KPD3(QObject *parent = Q_NULLPTR);

    ~KPD3();

    void step(double t, double dt) override;

    void setWheelDiameter(double wheel_diameter)
    {
        wheel_radius = wheel_diameter / 2.0;
    }

    void setOmega(double omega)
    {
        velocity = abs(omega) * wheel_radius;
        v_kmh = velocity * Physics::kmh;
    }

    double getVelocityKmh() const
    {
        return v_kmh;
    }

    void setCodeALSN(short code_alsn)
    {
        this->code_alsn = code_alsn;
    }

    bool getSafetyRelayState() const
    {
        return safety_relay.getState();
    }

private:

    double  velocity;

    double  v_kmh;

    double  wheel_radius;

    short   code_alsn;

    Timer   *checkTimer;

    bool    no_lock;

    Trigger safety_relay;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void preStep(state_vector_t &Y, double t);

    void stepExternalControl(double t, double dt);

private slots:

    void slotCheckTimer();
};


#endif // KPD3_H
