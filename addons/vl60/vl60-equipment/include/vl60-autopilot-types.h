#ifndef     VL60_AUTOPILOT_TYPES
#define     VL60_AUTOPILOT_TYPES

#include    <autopilot-types.h>
#include    <km-state.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class vl60_control_t : public auto_control_t
{
public:

    int km_pos_ref = POS_ZERO;

    /// позиция рукоятки крана машиниста
    int krm_pos = 1;

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

    /// Текущая тяговая позиция
    int cur_pos = 0;

    /// Текущая позициая рукоятки КМ
    int km_pos = POS_ZERO;    

    /// Текущий ток ТЭД
    double I_motor = 0;

    vl60_feedback_t() : auto_feedback_t()
    {

    }
};

#endif
