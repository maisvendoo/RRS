#ifndef MPCS_DATA_H
#define MPCS_DATA_H

#include    <array>
#include    "pant-description.h"

//------------------------------------------------------------------------------
// Структура входных сигналов
//------------------------------------------------------------------------------
struct mpcs_input_t
{
    /// Род тока
    int current_kind;

    /// Поднятые ТП
    std::array<bool, NUM_PANTOGRAPHS> pant_up;

    /// Опущеные ТП
    std::array<bool, NUM_PANTOGRAPHS> pant_down;

    /// Отключен ли БВ
    bool isOff_fs;

    /// ПРТ
    int current_kind_switch_state;

    /// Входное напряжение на ГВ
    double Uin_ms;

    /// Отключен ли ГВ
    bool isOff_ms;

    /// Входное напряжение на БВ
    double Uin_fs;

    std::array<double, 4> aux_const_U;

    // принимать данные давление в ГР
    /// Входное давление с главного резервуара
    double PressMR;

    mpcs_input_t()
    {
        current_kind = 0;
        std::fill(pant_up.begin(), pant_up.end(), false);
        std::fill(pant_down.begin(), pant_down.end(), false);

        isOff_fs = false;
        current_kind_switch_state = 0;
        Uin_ms = 0;

        isOff_ms = false;
        Uin_fs = 0;

        PressMR = 0;

        std::fill(aux_const_U.begin(), aux_const_U.end(), 0);
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct lamp_t
{
    float   state;
    bool    is_blinked;
    bool    blink_state;

    lamp_t()
        : state(0.0)
        , is_blinked(false)
        , blink_state(false)
    {

    }

    void blink(float state1, float state2)
    {
        if (is_blinked)
        {
            blink_state ? state = state1 : state = state2;
            blink_state = !blink_state;
        }
    }
};

//------------------------------------------------------------------------------
//  Состояние ламп сенсорных кнопок
//------------------------------------------------------------------------------
struct lamps_state_t
{
    lamp_t pant_fwd;
    lamp_t pant_bwd;
    lamp_t gv;
    lamp_t train_heating;
    lamp_t recup_disable;
    lamp_t auto_driver;
    lamp_t speed_control;
    lamp_t vz;

    lamp_t ept;
    lamp_t gs;
    lamp_t pv;
    lamp_t wheel_clean;
    lamp_t saund1;
    lamp_t brake_release;
    lamp_t test;
    lamp_t res_purge;
};

//------------------------------------------------------------------------------
// Структура выходных сигналов
//------------------------------------------------------------------------------
struct mpcs_output_t
{

    /// Включение ГВ
    bool turn_on_ms;

    /// Включение БВ
    bool turn_on_fs;

    /// Состояние мотор-копрессоров
    double MKstate;

    /// Положение выключателя "Цепи управления"
    float control_switch;

    /// Тумблеры управления мотор-компрессорами
    std::array<bool, 2> toggleSwitchMK;

    /// Состояние подсветки сенсорных клавиш
    lamps_state_t lamps_state;

    /// Состояние ТП
    std::array<bool, NUM_PANTOGRAPHS> pant_state;

    mpcs_output_t()
        : turn_on_ms(false)
        , turn_on_fs(false)
        , MKstate(0.0)
        , control_switch(0.0f)
    {
        std::fill(pant_state.begin(), pant_state.end(), false);
        std::fill(toggleSwitchMK.begin(), toggleSwitchMK.end(), false);
    }
};

#endif // MPCS_DATA_H

