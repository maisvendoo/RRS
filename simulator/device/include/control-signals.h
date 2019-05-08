#ifndef     CONTROL_SIGNALS_H
#define     CONTROL_SIGNALS_H

#include    <array>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    MAX_DISCRETE_CONTROL_SIGNALS = 1000,
    MAX_ANALOG_CONTROL_SIGNALS = 1000
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct control_signals_t
{
    std::array<bool, MAX_DISCRETE_CONTROL_SIGNALS>  discreteSignal;
    std::array<float, MAX_ANALOG_CONTROL_SIGNALS>   analogSignal;

    control_signals_t()
    {
        std::fill(discreteSignal.begin(), discreteSignal.end(), false);
        std::fill(analogSignal.begin(), analogSignal.end(), 0.0f);
    }
};

#endif // CONTROL_SIGNALS_H
