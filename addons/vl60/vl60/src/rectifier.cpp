#include    "rectifier.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Rectifier::Rectifier(QObject *parent) : Device(parent)
  , coeff(1.0)
  , U_in(0.0)
  , I_out(0.0)
  , U_out(0.0)
  , r(0.086)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Rectifier::~Rectifier()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Rectifier::setU_in(double value)
{
    U_in = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Rectifier::getU_out() const
{
    return U_out;
}

void Rectifier::setI_out(double value)
{
    I_out = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Rectifier::preStep(state_vector_t &Y, double t)
{
    U_out = coeff * U_in - getResist(U_in) * I_out;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Rectifier::ode_system(const state_vector_t &Y,
                           state_vector_t &dYdt,
                           double t)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Rectifier::load_config(CfgReader &cfg)
{
    cfg.getDouble("Device", "Coeff", coeff);

    QDomNode secNode = cfg.getFirstSection("InternalResistance");

    while (!secNode.isNull())
    {
        internal_resist_t int_res;

        cfg.getDouble(secNode, "U_in", int_res.U);
        cfg.getDouble(secNode, "r", int_res.r);

        internal_resist.push_back(int_res);

        secNode = cfg.getNextSection();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
internal_resist_t Rectifier::findResist(double u, internal_resist_t &next_res)
{
    internal_resist_t res;

    size_t left_idx = 0;
    size_t right_idx = internal_resist.size() - 2;
    size_t idx = (left_idx + right_idx) / 2;

    while (idx != left_idx)
    {
        internal_resist_t r = internal_resist[idx];

        if (u <= r.U)
            right_idx = idx;
        else
            left_idx = idx;

        idx = (left_idx + right_idx) / 2;
    }

    res = internal_resist[idx];
    next_res = internal_resist[idx+1];

    return res;
}

double Rectifier::getResist(double u)
{
    internal_resist_t res;
    internal_resist_t next_res;

    res = findResist(u, next_res);

    return res.r + (next_res.r - res.r) * (u - res.U) / (next_res.U - res.U);
}
