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

    /// положение КВТ
    double kvt_pos = 0;

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

    /// Состояние ЭПТ (Вкл/Выкл (или нет ЭПТ)
    bool is_EPB_on;

    /// Контроль состояния линейных контакторов
    bool is_LC_ON = false;

    vl60_feedback_t() : auto_feedback_t()
    {

    }
};

#endif
