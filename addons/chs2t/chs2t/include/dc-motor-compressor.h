#ifndef     DC_MOTOR_COMPRESSOR_H
#define     DC_MOTOR_COMPRESSOR_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DCMotorCompressor : public Device
{
public:

    DCMotorCompressor(QObject *parent = Q_NULLPTR);

    ~DCMotorCompressor();

    void setExternalPressure(double press) { p = press; }

    void setU(double value) { U = value; }

    void setPressure(double value) { p = value; }

    double getAirFlow() const { return Q; }

private:

    /// Внешнее противодавление
    double  p;

    /// Расход возхуа
    double  Q;

    double  U_power;
    double omega0;

    double  Mxx;
    double J;

    double R;
    double U;
    double cPhi;
    double I;
    double Ma;
    double Vnk;

    enum
    {
        NUM_COEFFS = 6
    };

    std::array<double, NUM_COEFFS>  K;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // DC_MOTOR_COMPRESSOR_H
