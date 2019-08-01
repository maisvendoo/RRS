#ifndef MPCS_DATA_H
#define MPCS_DATA_H

#include    <array>
#include    "pant-description.h"

//------------------------------------------------------------------------------
// Структура входных сигналов
//------------------------------------------------------------------------------
struct mpcs_input_t
{
    std::array<bool, NUM_PANTOGRAPHS> pant_up;

    std::array<bool, NUM_PANTOGRAPHS> pant_down;

    mpcs_input_t()
    {
        std::fill(pant_up.begin(), pant_up.end(), false);
        std::fill(pant_down.begin(), pant_down.end(), false);
    }
};

//------------------------------------------------------------------------------
// Структура выходных сигналов
//------------------------------------------------------------------------------
struct mpcs_output_t
{
    std::array<bool, NUM_PANTOGRAPHS> pant_state;

    mpcs_output_t()
    {
        std::fill(pant_state.begin(), pant_state.end(), false);
    }
};

#endif // MPCS_DATA_H
