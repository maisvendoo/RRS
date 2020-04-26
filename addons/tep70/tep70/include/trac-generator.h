#ifndef     TRAC_GENERATOR_H
#define     TRAC_GENERATOR_H

#include    "device.h"
#include    "motor-magnetic-char.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TracGenerator : public Device
{
public:

    TracGenerator(QObject *parent = Q_NULLPTR);

    ~TracGenerator();

    void setFieldVoltage(double Uf) { this->Uf = Uf; }

    void setLoadCurrent(double In) { this->In = In; }

    void setOmega(double omega) { this->omega = omega; }

    double getTorque() const { return M; }

    double getVoltage() const { return U; }

    double getFieldCurrent() const { return getY(0); }

    void load_marnetic_char(QString file_name);

    void load_eff_coeff(QString file_name);

private:

    double  Uf;

    double  In;

    double  omega;

    double  Ra;

    double  Rf;

    double  M;

    double  U;

    double  Tf;

    MotorMagneticChar   magnetic_char;

    MotorMagneticChar   eff_coef;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    double cPhi(double If);
};

#endif // TRACGENERATOR_H
