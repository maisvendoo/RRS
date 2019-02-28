#ifndef     PNEUMO_RELAY_H
#define     PNEUMO_RELAY_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT PneumoReley : public Device
{
public:

    PneumoReley(double work_volume = 2e-3, QObject *parent = Q_NULLPTR);

    virtual ~PneumoReley();

    void setWorkAirFlow(double flow);

    void setBrakeCylPressure(double pBC);

    void setPipelinePressure(double pPM);

    double getWorkPressure() const;

    double getBrakeCylAirFlow() const;

protected:

    double V_work;

    double K1;

    double K2;

    double Q_work;

    double pBC;

    double pPM;

    double Qbc;

    double k1;

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);

    virtual void preStep(state_vector_t &Y, double t);

    /// Load configuration
    virtual void load_config(CfgReader &cfg);
};

#endif // PNEUMO_RELAY_H
