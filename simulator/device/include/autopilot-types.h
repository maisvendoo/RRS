#ifndef     AUTOPILOT_TYPES_H
#define     AUTOPILOT_TYPES_H

#include    <ALSN-struct.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class auto_control_t
{
public:

    /// Активация нажатия РБ
    bool press_RB = false;

    /// Включение прожектора
    bool spotlight_ON = false;

    /// Включение подачи песка
    bool sand_ON = false;

    auto_control_t()
    {

    }

    virtual ~auto_control_t() = default;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class auto_feedback_t
{
public:

    /// Текущая скорость
    double v_cur = 0.0;
    /// Окружная скорость точек колеса
    double v_tau = 0.0;
    /// Текущее ограничение скорости
    double v_lim = 0;
    /// Следующее ограничение скорости
    double v_lim_next = 0;
    /// Дистанция до следующего ограничения скорости
    double limit_dist = 0;
    /// Код АЛСН
    ALSN alsn_code = NO_CODE;
    /// Дистанция до сигнала
    double signal_dist = 0;
    /// Сигнал проверки бдидельности
    bool is_vigilance_control = false;

    /// Давление в уравнительном резервуаре
    double pEQ = 0.0;
    /// Давление в тормозных цилиндрах
    double pBC = 0.0;
    /// Зарядное давление ТМ
    double p_charge = 0.0;

    auto_feedback_t()
    {

    }

    virtual ~auto_feedback_t() = default;
};

#endif
