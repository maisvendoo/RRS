//------------------------------------------------------------------------------
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     FEEDBACK_SIGNALS_H
#define     FEEDBACK_SIGNALS_H

#include    <array>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    MAX_DISCRETE_FEEDBACK_SIGNALS = 1000,
    MAX_ANALOG_FEEDBACK_SIGNALS = 1000
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct feedback_signals_t
{
    std::array<bool, MAX_DISCRETE_FEEDBACK_SIGNALS>  discreteSignal;
    std::array<float, MAX_ANALOG_FEEDBACK_SIGNALS>   analogSignal;

    feedback_signals_t()
    {
        std::fill(discreteSignal.begin(), discreteSignal.end(), false);
        std::fill(analogSignal.begin(), analogSignal.end(), 0.0f);
    }
};

#endif // FEEDBACK_SIGNALS_H
