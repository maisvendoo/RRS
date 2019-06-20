#ifndef PANTOGRAPH_H
#define PANTOGRAPH_H

#include    "device.h"

class Pantograph : public Device
{
public:

    Pantograph(QObject *parent = Q_NULLPTR);


    void setState(bool state);

    double getH();

    double getUout();

    void setUks(double _Uks);

private:
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);
    void load_config(CfgReader &cfg);
    void preStep(state_vector_t &Y, double t);
    void stepKeysControl(double t, double dt);



    CfgReader cfg_;

    double  tau;

    double  h;
    double hMax;

//    double  upTime;
//    double  downTime;

    double  V;

    double  Uout;
    double  Uks;

    bool state;

//    enum PantOrder{PANT_FORWARD,
//                  PANT_BACKWARD};

};

#endif // PANTOGRAPH_H
