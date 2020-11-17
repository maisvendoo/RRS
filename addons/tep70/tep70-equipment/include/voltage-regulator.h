#ifndef     VOLTAGE_REGULATOR_H
#define     VOLTAGE_REGULATOR_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class VoltageRegulator : public Device
{
public:

    VoltageRegulator(QObject *parent = Q_NULLPTR);

    ~VoltageRegulator();

    void setRefVoltage(double U_ref) { this->U_ref = U_ref; }

    void setVoltage(double U) { this->U = U; }

    void setBatteryVoltage(double U_bat) { this->U_bat = U_bat; }

    double getFieldVoltage() const { return Uf; }

private:

    double  U_ref;

    double  dU;

    double  U;

    double  Uf;

    double  U_bat;

    std::array<double , 2>  K;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};


#endif // VOLTAGE_REGULATOR_H
