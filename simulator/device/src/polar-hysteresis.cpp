#include    <polar-hysteresis.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PolarHysteresis::PolarHysteresis(double min_value,
                                 double max_value,
                                 int init_state,
                                 QObject *parent) : QObject(parent)
{
    min = min_value;
    max = max_value;

    state = init_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PolarHysteresis::~PolarHysteresis()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PolarHysteresis::setRange(double min_value, double max_value)
{
    min = min_value;
    max = max_value;
}
