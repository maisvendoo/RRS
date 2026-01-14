#ifndef     VL60_AUTOPILOT_TYPES
#define     VL60_AUTOPILOT_TYPES

#include    <autopilot-types.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class vl60_control_t : public auto_control_t
{
public:

    vl60_control_t() : auto_control_t()
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class vl60_feedback_t : public auto_feedback_t
{
public:

    vl60_feedback_t() : auto_feedback_t()
    {

    }
};

#endif
