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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    FB_READY = 0,
    FB_RBS = 1,
    FB_BRAKE_CRANE = 2,
    FB_LOCO_CRANE = 3,
    FB_RELEASE_VALVE = 4
};

Q_DECLARE_METATYPE(feedback_signals_t)

#endif // FEEDBACK_SIGNALS_H
