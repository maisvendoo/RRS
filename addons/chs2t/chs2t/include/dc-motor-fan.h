#ifndef DCMOTORFAN_H
#define DCMOTORFAN_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DCMotorFan : public Device
{
public:

    DCMotorFan(QObject *parent = Q_NULLPTR);

    ~DCMotorFan();

    void setU(double value) { U = value; }
    double getU() { return U; }

    void setSoundName(QString name) { soundName = name; }

private:

    double U;
    double cPhi;
    double R;
    double omega_nom;
    double ks;
    double J;

    QString soundName;

    double Unom;
    double kf;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // DCMOTORFAN_H
