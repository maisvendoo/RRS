#ifndef     KM2202
#define     KM2202

#include    "traction-controller.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    MIN_POSITION = 0,
    MAX_POSITION = 15
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ControllerKM2202 : public TractionController
{
public:

    ControllerKM2202(QObject *parent = Q_NULLPTR);

    ~ControllerKM2202();

private:

    int position;

    int reversor_dir;

    double trac_timeout;

    double revers_timeout;

    Timer   *incTracTimer;
    Timer   *decTracTimer;

    Timer   *incReversTimer;
    Timer   *decReversTimer;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);    

    void load_config(CfgReader &cfg);

    void stepKeysControl(double t, double dt);

private slots:

    void inc_trac_position();

    void dec_trac_position();

    void  inc_reversor_dir();

    void  dec_reversor_dir();

    void stepFeedback();
};

#endif
