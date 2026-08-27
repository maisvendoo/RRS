#ifndef     SL2M_H
#define     SL2M_H

#include    "device.h"
#include    "physics.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SL2M : public Device
{
public:

    SL2M(QObject *parent = nullptr);

    ~SL2M();

    /// Задать угловую скорость вращения колесной пары, рад./с
    void setOmega(double value);

    /// Задать диаметр колесной пары, м
    void setWheelDiameter(double d);

    /// Скорость, м/с
    double getVelocity() const;

    /// Угол поворота стрелки указателя скорости, д.е.
    float getArrowPos() const;

    /// Угол поворота вала привода, рад.
    float getShaftPos() const;

    /// Состояние звука работы часового механизма
    sound_state_t getSoundState(size_t idx = 0) const override;

    /// Сигнал состояния звука работы часового механизма
    float getSoundSignal(size_t idx = 0) const override;

private:

    enum
    {
        SHAFT_ANGLE,            ///< Y[0] угол поворота вала привода, рад.
        VELOCITY_CURRENTSEGMENT,///< Y[1] скорость, регистрируемая подъёмом текущего сегмента, м/с
        TIMER_NEXT_SEGMENT      ///< Y[2] счётчик времени до поворота к следующему сегменту, с
    };

    /// Скорость, зафиксированная подъёмом предыдущего сегмента, м/с
    double velocity_previous_segment = 0.0;

    /// Скорость, м/с
    double velocity = 0.0;

    /// Угловая скорость вращения колесной пары, рад./с
    double omega = 0.0;

    /// Шаг засечек для фиксации скорости на сегменте, м/с
    float velocity_segment_step = 2.5 / Physics::kmh;

    /// Максимальная скорость на шкале, м/с
    float max_speed = 150.0 / Physics::kmh;

    /// Скорость начала работы звука скоростемера, м/с
    float speed_begin_sound = 2.0 / Physics::kmh;

    /// Время подъёма текущего сегмента до фиксации и переключения на следующий, с
    float cycle_time = 1.0;

    /// Радиус колесной пары, м
    float r_wheel = 1.25 / 2.0;

    /// Номинальный радиус колесной пары, м
    float r_nominal = 1.18 / 2.0;

    /// Использовать номинальный расчётный диаметр колеса вместо реального,
    /// с искажением скорости в зависимости от износа бандажа, как в реальности
    bool use_nominal_diameter = true;

    /// Частота отклонения зафиксированной скорости на шаг в большую сторону, 0 - 100
    std::uint8_t velocity_random_plus = 0;
    /// Частота отклонения зафиксированной скорости на шаг в меньшую сторону, 0 - 100
    std::uint8_t velocity_random_minus = 0;

    /// LCG-генератор псевдослучайных чисел от 0 до 255
    std::uint8_t random_value = 0;
    std::uint8_t random_next();

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) override;

    void load_config(CfgReader &cfg) override;
};

#endif // SL2M_H
