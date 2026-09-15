#ifndef     MSUD_DATA_H
#define     MSUD_DATA_H

#include    <array>
#include    "physics.h"
#include    "shunts-module-defines.h"

// Режимы работы статуса оборудования
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    MSUD_STATUS_OFF = 0,
    MSUD_STATUS_ON = 1,
    MSUD_STATUS_BLINK = 2
};

// Доступные состояния МСУД
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum msud_state_t : std::uint8_t
{
    MSUD_OFF = 0,
    MSUD_RESET = 1,
    MSUD_READY = 2
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    TRAC_MOTORS_NUM = 6,
    TRAC_MOTOR1 = 0,
    TRAC_MOTOR2 = 1,
    TRAC_MOTOR3 = 2,
    TRAC_MOTOR4 = 3,
    TRAC_MOTOR5 = 4,
    TRAC_MOTOR6 = 5
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    MOTOR_FANS_NUM = 4,
    MV1 = 0,
    MV2 = 1,
    MV3 = 2,
    MV4 = 3
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct msud_input_t
{
    /// Положение тумблера (МПК1/МПК2)
    bool tumbler_MPK = false;

    /// Признак включения ПЧФ
    bool is_PCHF_On = false;

    /// Режим управления (Авторегулирование/Ручной)
    bool is_auto_reg = false;

    /// Признак сбора тяги
    bool is_traction = false;

    /// Признак сбора рекуперации
    bool is_brake = false;

    /// Признак экстренного торможения
    bool is_emergency_brake = false;

    /// Положение главного вала КМ в тяге
    double km_trac_level = 0.0;

    /// Положение главного вала КМ в рекуперации
    double km_brake_level = 0.0;

    /// Положение задатчика скорости
    double km_ref_velocity_level = 0.0;

    /// Текущая скорость электровоза, км/ч
    double V_cur = 0.0;

    /// Токи якоря тяговых двигателей
    std::array<double, TRAC_MOTORS_NUM> Ia = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    /// Токи возбуждения тяговых двигателей
    std::array<double, TRAC_MOTORS_NUM> If = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    /// Состояние мотор-вентиляторов
    std::array<bool, MOTOR_FANS_NUM> mv_state = {false, false, false, false};

    /// Давление в ТЦ тележек
    std::array<double, TRAC_MOTORS_NUM / 2> TC_press = {0.0, 0.0, 0.0};
};


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct msud_output_t
{
    msud_state_t state = MSUD_OFF;

    /// Подача питания на реле КМ23
    bool kv23_On = false;

    /// Сигнал включения реле KV14
    bool kv14_On = false;

    /// Работа МВ на низкой частоте
    bool is_MV_low_freq = false;

    /// Сигнал отсутствия усиления торможения от вентиля Y5
    bool is_not_brake_boost = true;

    /// Индикация наличия давления в тормозных цилиндрах
    std::uint8_t TC_status = 0;

    /// Номер зоны ВИП
    std::uint8_t zone_num = 1;

    /// Ступень ослабления возбуждения
    std::uint8_t field_weak_step = 0;

    /// Минимальное давление в тормозных цилиндрах
    double TC_min_press = 0.11;

    /// Уровень напряжение на выходе ВИП (от 0.0 до 4.0) отображаемый на БИ
    double vip_voltage_level = 0.0;

    /// Угол открытия тиристоров
    double alpha = Physics::PI / 2.0;

    /// Уровень тока возбуждения от ВУВ
    double field_level = 0.0;

    /// Ограничение тока ББР
    double Ib_max = 950.0;

    /// Ограниение тока возбуждения
    double If_max = 845.0;

    /// Ограничение тока якоря
    double Ia_max = 1300.0;

    /// Максимальная задаваемая скорость
    double Vmax = 140.0;

    /// Включение МВ на низкой частоте
    std::array<bool, MOTOR_FANS_NUM> mv_freq_low = {true, true, true, true};
    /// Включение МВ на нормальной частоте
    std::array<bool, MOTOR_FANS_NUM> mv_freq_norm = {false, false, false, false};
    /// Наполнение ТЦ тележек
    std::array<bool, TRAC_MOTORS_NUM / 2> TC_full = {false, false, false};

    /// Сигнализация супеней ОП
    std::array<bool, NUM_STEPS> op = {false, false, false};
};

#endif // MSUD_DATA_H
