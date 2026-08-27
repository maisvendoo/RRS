#include    "sl2m.h"

/// Передаточное число червячного редуктора, ед.
constexpr double ip = Physics::PI * 3.0 / 50.0;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SL2M::SL2M(QObject *parent) : Device(parent)
{
    random_value = static_cast<std::uint8_t>(std::time(0) & 0xFF);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SL2M::~SL2M()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SL2M::setOmega(double value)
{
    omega = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SL2M::setWheelDiameter(double d)
{
    r_wheel = d / 2.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SL2M::getVelocity() const
{
    return velocity;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float SL2M::getArrowPos() const
{
    return static_cast<float>(velocity) / max_speed;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float SL2M::getShaftPos() const
{
    return static_cast<float>(getY(SHAFT_ANGLE));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t SL2M::getSoundState(size_t idx) const
{
    (void) idx;
    return sound_state_t(velocity >= speed_begin_sound);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float SL2M::getSoundSignal(size_t idx) const
{
    (void) idx;
    return sound_state_t::createSoundSignal(velocity >= speed_begin_sound);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::uint8_t SL2M::random_next()
{
    random_value = static_cast<std::uint8_t>((101 * random_value + 103) & 0xFF);
    return random_value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SL2M::preStep(state_vector_t &Y, double t)
{
    // Подъём фиксирующего скорость сегмента ограничен
    while (Y[VELOCITY_CURRENTSEGMENT] > max_speed)
    {
        Y[VELOCITY_CURRENTSEGMENT] -= velocity_segment_step;
    }

    // Скорость, максимальная из зарегистрированных текущим или предыдущим сегментом
    velocity = std::max(Y[VELOCITY_CURRENTSEGMENT], velocity_previous_segment);

    // Раз в секунду - переход к следующему сегменту
    if (Y[TIMER_NEXT_SEGMENT] >= cycle_time)
    {
        // Обнуляем таймер
        Y[TIMER_NEXT_SEGMENT] = 0.0;

        // Количество засечек, на которое поднялся сегмент за прошедшую секунду
        double step_num = std::round(Y[VELOCITY_CURRENTSEGMENT] / velocity_segment_step);

        // Случайный перескок на соседнюю засечку
        if ((velocity_random_plus || velocity_random_minus) && (step_num > 1.0))
        {
            random_next();
            if (random_value < velocity_random_minus)
            {
                // Выпало маленькое число - бросок в минус
                step_num -= 1.0;
            }
            else if (random_value > (255 - velocity_random_plus))
            {
                // Выпало большое число - бросок в плюс
                step_num += 1.0;
            }
        }

        // Следующую секунду этот сегмент будет удерживаться фиксирующим роликом
        velocity_previous_segment = step_num * velocity_segment_step;

        // А подниматься для фиксации скорости будет очередной сегмент, с нуля
        Y[VELOCITY_CURRENTSEGMENT] = 0.0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SL2M::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
    (void)Y;
    (void)t;

    // Настройка: расчёт скорости по реальному диаметру колеса с учётом износа,
    // либо по расчётному диаметру, с соответствующим искажением, как в реальности
    const double r_calc = use_nominal_diameter ? r_nominal : r_wheel;

    // Вращение вала привода от колеса через червячный редуктор
    dYdt[SHAFT_ANGLE] = omega * ip;

    // Подъём текущего сегмента для фиксации скорости за последнюю секунду
    dYdt[VELOCITY_CURRENTSEGMENT] = std::abs(omega) * r_calc / cycle_time;

    // Таймер. Просто таймер.
    dYdt[TIMER_NEXT_SEGMENT] = 1.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SL2M::load_config(CfgReader &cfg)
{
    QString secName = "Device";
    double tmp;
    int random_shift;

    tmp = 0.0;
    cfg.getDouble(secName, "MaxSpeed", tmp);
    if (tmp > Physics::ZERO)
    {
        max_speed = tmp / Physics::kmh;
    }

    tmp = 0.0;
    cfg.getDouble(secName, "SpeedStep", tmp);
    if (tmp > Physics::ZERO)
    {
        velocity_segment_step = tmp / Physics::kmh;
    }

    tmp = 0.0;
    cfg.getDouble(secName, "SoundSpeed", tmp);
    if (tmp > Physics::ZERO)
    {
        speed_begin_sound = tmp / Physics::kmh;
    }

    tmp = 0.0;
    cfg.getDouble(secName, "CycleTime", tmp);
    if (tmp > Physics::ZERO)
    {
        cycle_time = tmp;
    }

    tmp = 0.0;
    cfg.getDouble(secName, "WheelNominalDiameter", tmp);
    if (tmp > Physics::ZERO)
    {
        r_nominal = tmp / 2000.0;
    }

    cfg.getBool(secName, "UseNominalDiameter", use_nominal_diameter);

    random_shift = 0;
    cfg.getInt(secName, "VelocityRandomPlus", random_shift);
    if ((random_shift > 0) && (random_shift <= 100))
    {
        velocity_random_plus = random_shift;
    }

    random_shift = 0;
    cfg.getInt(secName, "VelocityRandomMinus", random_shift);
    if ((random_shift > 0) && (random_shift <= 100))
    {
        velocity_random_minus = random_shift;
    }
}
