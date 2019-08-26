#ifndef MPCS_DATA_H
#define MPCS_DATA_H

#include    <array>
#include    "pant-description.h"

//------------------------------------------------------------------------------
// Структура входных сигналов
//------------------------------------------------------------------------------
struct mpcs_input_t
{
    int current_kind;

    std::array<bool, NUM_PANTOGRAPHS> pant_up;

    std::array<bool, NUM_PANTOGRAPHS> pant_down;


    /// Отключен ли БВ
    bool isOff_fs;

    /// ПРТ
    int current_kind_switch_state;

    /// Входное напряжение на ГВ
    double Uin_ms;

    /// Нажатие кнопки
    bool pressedButton;

    /// Отключен ли ГВ
    bool isOff_ms;

    /// Входное напряжение на БВ
    double Uin_fs;

    mpcs_input_t()
    {
        current_kind = 0;
        std::fill(pant_up.begin(), pant_up.end(), false);
        std::fill(pant_down.begin(), pant_down.end(), false);

        isOff_fs = false;
        current_kind_switch_state = 0;
        Uin_ms = 0;
        pressedButton = false;

        isOff_ms = false;
        Uin_fs = 0;
    }
};

//------------------------------------------------------------------------------
// Структура выходных сигналов
//------------------------------------------------------------------------------
struct mpcs_output_t
{
    std::array<bool, NUM_PANTOGRAPHS> pant_state;

    /// Включение ГВ
    bool turn_on_ms = false;

    /// Включение БВ
    bool turn_on_fs = false;

    mpcs_output_t()
    {
        std::fill(pant_state.begin(), pant_state.end(), false);
    }
};

#endif // MPCS_DATA_H
