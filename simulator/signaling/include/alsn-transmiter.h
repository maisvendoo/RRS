#ifndef     ALSN_TRANSMITER_H
#define     ALSN_TRANSMITER_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    RED_ALSN = 1,
    YELLOW_ALSN = 2,
    GREEN_ALSN = 3
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Transmiter : public Device
{
    Q_OBJECT

public:

    Transmiter(QObject *parent = Q_NULLPTR);

    ~Transmiter();

    void setState(bool is_red, bool is_green, bool is_yellow);

signals:

    void sendAlsnCode(int alsn_code);

private:

    bool is_red;

    bool is_green;

    bool is_yellow;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t);


    void load_config(CfgReader &cfg);
};

#endif // ALSNTRANSMITER_H
