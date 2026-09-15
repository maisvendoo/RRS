#ifndef     EP1M_AUTOPILOT_TYPES_H
#define     EP1M_AUTOPILOT_TYPES_H

#include    <autopilot-types.h>
#include    <cstdint>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ep1m_control_t : public auto_control_t
{
public:

    int8_t mode_pos = 0;

    double level = 0.0;

    double v_level = 0.0;

    int krm_pos = 1;

    double kvt_pos = 0.0;

    ep1m_control_t()
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ep1m_feedback_t : public auto_feedback_t
{
public:

    /// Ток якоря ТЭД
    double I_motor = 0.0;

    /// Признак активности ЭПТ
    bool is_EPB_on = false;

    /// Признак замыкания линейных контакторов ТЭД
    bool is_LC_ON = false;

    /// Признак сбора тяги
    bool is_traction_ON = false;

    /// Признак сбора рекуперации
    bool is_brake_ON = false;

    /// Признак нулевого положения КМ
    bool km_is_zero = false;

    ep1m_feedback_t()
    {

    }
};

#endif
