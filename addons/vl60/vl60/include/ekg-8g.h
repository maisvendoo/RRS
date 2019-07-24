#ifndef     EKG_8G_H
#define     EKG_8G_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class EKG_8G : public Device
{
public:

    EKG_8G(QObject *parent = Q_NULLPTR);

    ~EKG_8G();

private:

    enum
    {
        NUM_POSITIONS = 38
    };

    /// Номер текущей позиции
    size_t  position;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // EKG_8G_H
