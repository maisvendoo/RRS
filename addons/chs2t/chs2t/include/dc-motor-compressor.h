#ifndef     DC_MOTOR_COMPRESSOR_H
#define     DC_MOTOR_COMPRESSOR_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DCMotorCompressor : public Device
{
public:

    DCMotorCompressor(QString config_path, QObject *parent = Q_NULLPTR);

    ~DCMotorCompressor();

    void setExternalPressure(double press);

    double getAirFlow() const;

    void setU(double value) { U = value; };

private:
//    double  Mmax;

//    double  s_kr;

//    double  Mxx;

//    double  Vnk;

    /// Внешнее противодавление
    double  p;

    /// Расход возхуа
    double  Q;

    double  U_power;
    double omega0;

    double R;
    double U;
    double cPhi;
    double I;
    double Ma;


    enum
    {
        NUM_COEFFS = 6
    };

    std::array<double, NUM_COEFFS>  K;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void load_config(QString cfg_path);
};

#endif // DC_MOTOR_COMPRESSOR_H
