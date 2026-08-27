#include    "sl2m.h"

#include    "physics.h"

#include    <QRandomGenerator>
#include    <algorithm>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SL2M::SL2M(QObject *parent) : Device(parent)
    , omega(0.0)
    , ip(3.0 * Physics::PI / 50.0)
    , max_speed(150.0)
    , arrow_pos(0.0f)
    , Dk(1.25)
    , speed_begin_sound(2.0)
    , omega_begin_sound(speed_begin_sound * 2.0 / Dk / Physics::kmh)
    , shaft_pos(0.0f)
    , cycle_time(3.0)
    , phase(0.0)
    , t_prev(0.0)
    , gear_wear(0.0)
    , holding_slip(0.0)
    , clock_jitter(0.0)
    , fall_speed(0.5)
    , phase_velocity(1.0 / 3.0)
    , shaft_angle(0.0)
{
    seg_height[0] = 0.0;
    seg_height[1] = 0.0;
    seg_height[2] = 0.0;

    lifting_idx = 0;
    holding_idx = 1;
    falling_idx = 2;

    y.resize(0);
    dydt.resize(0);
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
void SL2M::setWheelDiameter(double diam)
{
    Dk = diam;
    omega_begin_sound = speed_begin_sound * 2.0 / Dk / Physics::kmh;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float SL2M::getArrowPos() const
{
    return arrow_pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float SL2M::getShaftPos() const
{
    return shaft_pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t SL2M::getSoundState(size_t idx) const
{
    (void) idx;
    return sound_state_t(abs(omega) >= omega_begin_sound);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float SL2M::getSoundSignal(size_t idx) const
{
    (void) idx;
    return sound_state_t::createSoundSignal(abs(omega) >= omega_begin_sound);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SL2M::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)

    double dt = t - t_prev;
    t_prev = t;

    if (dt <= 0.0)
        return;

    double roll_omega = fabs(omega);

    velocity = roll_omega * Dk / 2.0 * Physics::kmh;

    shaft_angle += ip * omega * dt;
    shaft_pos = static_cast<float>(shaft_angle / 2.0 / Physics::PI);

    double d_phase = phase_velocity * dt;

    if (clock_jitter > 0.0)
    {
        double noise = QRandomGenerator::global()->generateDouble() * 2.0 - 1.0;
        d_phase *= (1.0 + clock_jitter * noise);
    }

    double old_phase = phase;
    phase += d_phase;

    if (phase >= 1.0)
        phase -= 1.0;

    double p1 = 1.0 / 3.0;
    double p2 = 2.0 / 3.0;

    double cross_1 = (old_phase < p1 && phase >= p1) || (old_phase < p1 && phase < old_phase);
    double cross_2 = (old_phase < p2 && phase >= p2) || (old_phase < p2 && phase < old_phase && phase < p1);
    double cross_0 = (old_phase >= p2 && phase < p1) || (phase < old_phase && old_phase >= p2);

    if (cross_1 || cross_2 || cross_0)
    {
        seg_height[falling_idx] = 0.0;

        int tmp = lifting_idx;
        lifting_idx = falling_idx;
        falling_idx = holding_idx;
        holding_idx = tmp;
    }

    double lift_duration = cycle_time / 3.0;
    double target_height = velocity / max_speed;
    seg_height[lifting_idx] += (target_height / lift_duration) * dt * (1.0 - gear_wear);
    seg_height[lifting_idx] = std::min(seg_height[lifting_idx], 1.0);

    if (holding_slip > 0.0)
    {
        seg_height[holding_idx] -= holding_slip * dt;
        seg_height[holding_idx] = std::max(seg_height[holding_idx], 0.0);
    }

    seg_height[falling_idx] -= fall_speed * dt;
    seg_height[falling_idx] = std::max(seg_height[falling_idx], 0.0);

    arrow_pos = static_cast<float>(seg_height[holding_idx]);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SL2M::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
    Q_UNUSED(t)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SL2M::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "MaxSpeed", max_speed);

    cfg.getDouble(secName, "SoundSpeed", speed_begin_sound);

    cfg.getDouble(secName, "CycleTime", cycle_time);
    if (cycle_time > 0.0)
        phase_velocity = 1.0 / cycle_time;

    cfg.getDouble(secName, "GearWear", gear_wear);
    gear_wear = std::clamp(gear_wear, 0.0, 1.0);

    cfg.getDouble(secName, "HoldingSlip", holding_slip);
    holding_slip = std::max(holding_slip, 0.0);

    cfg.getDouble(secName, "ClockJitter", clock_jitter);
    clock_jitter = std::max(clock_jitter, 0.0);

    cfg.getDouble(secName, "FallSpeed", fall_speed);
    fall_speed = std::max(fall_speed, 0.0);
}