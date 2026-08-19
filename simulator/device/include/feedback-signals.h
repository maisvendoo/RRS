//------------------------------------------------------------------------------
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     FEEDBACK_SIGNALS_H
#define     FEEDBACK_SIGNALS_H

#include    <array>
#include    "external-signal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct feedback_signals_t
{
    enum
    {
        MAX_FEEDBACK_SIGNALS = 1000
    };

    std::array<signal_t, MAX_FEEDBACK_SIGNALS>   analogSignal;

    feedback_signals_t()
    {

    }
};

Q_DECLARE_METATYPE(feedback_signals_t)

#endif // FEEDBACK_SIGNALS_H
