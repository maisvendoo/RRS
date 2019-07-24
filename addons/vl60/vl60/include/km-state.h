#ifndef     KM_STATE_H
#define     KM_STATE_H

#include    <array>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct km_state_t
{
    /// Заданное контроллером направление движения
    int     revers_ref_state;

    enum
    {
        NUM_MAIN_POSITIONS = 8,
        POS_BV = 0,
        POS_ZERO = 1,
        POS_AV = 2,
        POS_RV = 3,
        POS_FV = 4,
        POS_FP = 5,
        POS_RP = 6,
        POS_AP = 7
    };

    /// Ступень ослабления возбуждения
    int     field_loosen_pos;

    /// Состояние каждой из позиций главной рукоятки
    std::array<bool, NUM_MAIN_POSITIONS>    pos_state;


    km_state_t()
        : revers_ref_state(0)
        , field_loosen_pos(0)
    {
        std::fill(pos_state.begin(), pos_state.end(), false);
        pos_state[POS_ZERO] = true;
    }
};

#endif // KM_STATE_H
