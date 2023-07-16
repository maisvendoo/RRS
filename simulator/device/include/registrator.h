#ifndef     REGISTRATOR_H
#define     REGISTRATOR_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT Registrator : public Device
{
    Q_OBJECT

public:

    Registrator(double interval = 0.1, QObject *parent = Q_NULLPTR);

    ~Registrator();

    virtual void print(QString line, double t, double dt);

protected:

    bool    first_print;
    double  tau;
    double  interval;
    QString fileName;
    QString path;

    QFile   *file;

    virtual void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    virtual void load_config(CfgReader &cfg);
};


#endif // REGISTRATOR_H
