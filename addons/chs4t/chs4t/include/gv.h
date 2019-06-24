#ifndef GV_H
#define GV_H

#include    "device.h"

class GV : public Device
{
public:

    GV(QObject *parent = Q_NULLPTR);



    double getX();
    double getUout();

    void setP0(double _P0);
    void setP1(double _P1);

    void setUkr(double Ukr);

    void setState(bool state);

    void setPhc(bool phc);

private:
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);
    void load_config(CfgReader &cfg);
    void preStep(state_vector_t &Y, double t);
    void stepKeysControl(double t, double dt);



    CfgReader cfg_;
    std::vector<double> Y;

    double  Uout;
    double  Ukr;

    double Vn;
    double Vdk;
    double Fk;
    double Fp;
    double P0;
    double P1;
    double K1;
    double K2;

    double x;
    double x2;
    double sdk;
    double Pdk;


    bool state;
    ///
    bool phc;

};

#endif // GV_H