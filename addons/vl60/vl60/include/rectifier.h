#ifndef     RECTIFIER_H
#define     RECTIFIER_H

#include    "device.h"

struct internal_resist_t
{
    double U;
    double r;

    internal_resist_t()
        : U(0)
        , r(0)
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Rectifier : public Device
{
public:

    Rectifier(QObject *parent = Q_NULLPTR);

    ~Rectifier();

    void setU_in(double value);

    double getU_out() const;

    void setI_out(double value);

private:

    /// Коэффициент, связывающией входное и выходное напряжение
    double coeff;

    double U_in;

    double I_out;

    double U_out;

    double r;

    std::vector<internal_resist_t> internal_resist;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    internal_resist_t findResist(double u, internal_resist_t &next_res);

    double getResist(double u);
};

#endif // RECTIFIER_H
