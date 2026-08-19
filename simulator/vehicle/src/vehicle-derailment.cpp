//------------------------------------------------------------------------------
//
//      Derailment system (постепенный сход с рельсов)
//
//------------------------------------------------------------------------------

#include    "vehicle-derailment.h"

#include    "vehicle-dynamics.h"
#include    "vehicle-lateral-dynamics.h"

#include    <CfgReader.h>
#include    <Journal.h>

#include    <algorithm>
#include    <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleDerailment::VehicleDerailment() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleDerailment::loadConfig(QString cfg_path, std::size_t num_axis,
                                   std::size_t vehicle_idx_)
{
    vehicle_idx = vehicle_idx_;

    axle_overload_timer.assign(num_axis, 0.0);
    derailed_axles.assign(num_axis, false);

    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "Derailment";

    cfg.getBool(sec, "Enabled", enabled);
    cfg.getDouble(sec, "YQLimit", yq_limit_override);
    cfg.getDouble(sec, "WheelUnloadLimit", wheel_unload_limit);
    cfg.getDouble(sec, "FlangeClimbTime", flange_climb_time);
    cfg.getDouble(sec, "DerailDisplacement", derail_displacement);
    cfg.getDouble(sec, "DerailedResistance", derailed_resistance);
    cfg.getDouble(sec, "LateralImpactThreshold", lateral_impact_threshold);
    cfg.getDouble(sec, "MassCenterHeight", mass_center_height);
    cfg.getDouble(sec, "RolloverArm", rollover_arm);
    cfg.getDouble(sec, "ResistanceSleepers", resistance_sleepers);
    cfg.getDouble(sec, "ResistanceBallast", resistance_ballast);
    cfg.getDouble(sec, "ResistanceGround", resistance_ground);
    cfg.getDouble(sec, "SleeperSpacing", sleeper_spacing);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleDerailment::isEnabled() const
{
    return enabled;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleDerailment::step(double dt,
                             const VehicleLateralDynamics& lateral,
                             const VehicleVerticalDynamics& vertical,
                             std::size_t num_axis)
{
    if (!enabled)
        return;

    if (derailed_axles.size() != num_axis)
    {
        axle_overload_timer.assign(num_axis, 0.0);
        derailed_axles.assign(num_axis, false);
    }

    // Сход необратим без явного сброса (ремонт/восстановление)
    if (state == State::VehicleDerailed)
        return;

    bool any_axle = false;
    bool warning = false;

    // Принудительная разгрузка колёс после бокового удара
    const bool impact_unload = (forced_unload_timer > 0.0);
    forced_unload_timer = std::max(0.0, forced_unload_timer - dt);

    for (std::size_t i = 0; i < num_axis; ++i)
    {
        if (derailed_axles[i])
        {
            any_axle = true;
            continue;
        }

        // Худшее колесо оси по Y/Q и смещение оси к гребню
        const double yq_left = lateral.getWheelYQ(i, 0);
        const double yq_right = lateral.getWheelYQ(i, 1);
        const double yq = std::max(yq_left, yq_right);

        const double y_disp = std::abs(lateral.getWheelsetLateralPosition(i));

        // Разгрузка колеса от вертикальной динамики или бокового удара
        const double load = impact_unload ? 0.0
                                          : vertical.getAxleLoadFactor(i);

        // Предел: переопределённый или типовой предел Надаля
        const double limit = (yq_limit_override > 0.05)
                ? yq_limit_override
                : 0.8;

        // Подъём гребня: устойчивое превышение Y/Q у гребня ИЛИ
        // почти разгруженное колесо, доведённое до гребня (ТЗ, п.12:
        // рост боковых сил -> разгрузка колеса -> подъём гребня)
        const bool overloaded = (yq > limit) &&
                (y_disp > 0.5 * derail_displacement);

        const bool wheel_unloaded = (load < wheel_unload_limit) &&
                (y_disp > derail_displacement);

        if (overloaded || wheel_unloaded)
        {
            axle_overload_timer[i] += dt;
            warning = true;
        }
        else
        {
            axle_overload_timer[i] = std::max(0.0, axle_overload_timer[i] - 2.0 * dt);
        }

        // Устойчивое превышение: гребень перелез через рельс - ось сошла
        if (axle_overload_timer[i] >= flange_climb_time)
        {
            derailed_axles[i] = true;
            any_axle = true;

            Journal::instance()->critical(QString(
                "[DERAILMENT] Vehicle #%1 axle %2 DERAILED "
                "(Y/Q sustained, wheel climb)")
                .arg(static_cast<int>(vehicle_idx))
                .arg(static_cast<int>(i)));
        }
    }

    // Продвижение стадии
    if (any_axle)
    {
        // Число сошедших осей
        const std::size_t derailed_count = static_cast<std::size_t>(
                    std::count(derailed_axles.begin(), derailed_axles.end(), true));

        state = State::AxleDerailed;

        // Обе крайние оси или половина осей - считаем тележку сошедшей
        if (derailed_count >= std::max<std::size_t>(num_axis / 2, 1))
        {
            state = State::BogieDerailed;
        }

        if (derailed_count >= num_axis)
        {
            state = State::VehicleDerailed;
        }
    }
    else if (warning)
    {
        state = State::WheelLiftWarning;
    }
    else if (state == State::WheelLiftWarning)
    {
        state = State::OnTrack;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleDerailment::State VehicleDerailment::getState() const
{
    return state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleDerailment::isDerailed() const
{
    return state >= State::AxleDerailed;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleDerailment::isVehicleDerailed() const
{
    return state == State::VehicleDerailed;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const std::vector<bool>& VehicleDerailment::getDerailedAxles() const
{
    return derailed_axles;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleDerailment::applyLateralImpact(double energy,
                                           double nx, double ny, double nz,
                                           double ox, double oy, double oz,
                                           double o_len)
{
    if (o_len < 1e-6)
        return;

    // Доля энергии в боковом (непродольном) направлении
    const double cos_a = (nx * ox + ny * oy + nz * oz) / o_len;
    const double sin2 = std::max(0.0, 1.0 - cos_a * cos_a);
    const double lateral_energy = energy * sin2;

    if (lateral_energy < lateral_impact_threshold)
        return;

    // Длительность разгрузки растёт с энергией удара (0.1..0.5 с)
    forced_unload_timer = std::min(0.5, 0.1 + lateral_energy / 2.0e6);

    Journal::instance()->warning(QString(
        "[DERAILMENT] Vehicle #%1 lateral impact %2 kJ: wheels unloaded for %3 s")
        .arg(static_cast<int>(vehicle_idx))
        .arg(lateral_energy / 1000.0, 0, 'f', 0)
        .arg(forced_unload_timer, 0, 'f', 2));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleDerailment::setMassCenterHeight(double height)
{
    mass_center_height = std::max(height, 0.5);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleDerailment::stepRollover(double dt, double velocity,
                                     double lateral_accel)
{
    last_velocity = velocity;

    // Частота ударов о шпалы при движении после схода (для звука/VFX)
    // рассчитывается из шага шпал и скорости

    // Физический критерий опрокидывания: боковое ускорение на плече
    // центра масс против силы тяжести на плече колеи (ТЗ, п.10):
    // опрокидывание при a_lat * h_cm > g * arm
    const double g = 9.81;
    const double demand = std::abs(lateral_accel) * mass_center_height;
    const double capacity = g * rollover_arm;

    const double margin = (capacity - demand) / capacity;

    if (!isDerailed() && rollover_state < RolloverState::Partial)
    {
        // На рельсах оцениваем только предупреждение
        rollover_state = (margin < 0.15) ? RolloverState::Warning
                                         : RolloverState::None;
        rollover_timer = 0.0;
        return;
    }

    if (margin < -0.05)
    {
        // За пределом устойчивости: частичное -> полное за 0.5 с
        rollover_timer += dt;

        if (rollover_timer > 0.5)
        {
            if (rollover_state != RolloverState::Full)
            {
                rollover_state = RolloverState::Full;

                Journal::instance()->critical(QString(
                    "[DERAILMENT] Vehicle #%1 ROLLED OVER")
                    .arg(static_cast<int>(vehicle_idx)));
            }
        }
        else
        {
            rollover_state = RolloverState::Partial;
        }
    }
    else if (margin < 0.15)
    {
        rollover_state = RolloverState::Warning;
        rollover_timer = 0.0;
    }
    else
    {
        // Частичное опрокидывание обратимо до полного
        if (rollover_state == RolloverState::Warning)
        {
            rollover_state = RolloverState::None;
        }
        rollover_timer = 0.0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleDerailment::RolloverState VehicleDerailment::getRolloverState() const
{
    return rollover_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleDerailment::isRollover() const
{
    return rollover_state == RolloverState::Full;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleDerailment::SurfaceType VehicleDerailment::getSurfaceType() const
{
    return surface;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleDerailment::setSurfaceType(SurfaceType surface_)
{
    surface = surface_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleDerailment::getSleeperImpactRate() const
{
    if (!isDerailed() || sleeper_spacing < 0.05)
        return 0.0;

    return std::abs(last_velocity) / sleeper_spacing;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleDerailment::getResistanceCoefficient() const
{
    if (!isDerailed())
        return 0.0;

    // Полное опрокидывание: скольжение кузовом
    if (isRollover())
        return 0.6;

    // Сопротивление зависит от доли сошедших осей и поверхности
    // (ТЗ "Физика после схода", п.6): 1 пара - умеренное, вся ПЕ - большое
    const double derailed_share = static_cast<double>(
                std::count(derailed_axles.begin(), derailed_axles.end(), true)) /
            std::max<double>(static_cast<double>(derailed_axles.size()), 1.0);

    double base = derailed_resistance;

    switch (surface)
    {
    case SurfaceType::Sleepers:
        base = resistance_sleepers;
        break;
    case SurfaceType::Ballast:
        base = resistance_ballast;
        break;
    case SurfaceType::Ground:
        base = resistance_ground;
        break;
    }

    // Одна ось из четырёх даёт четверть полного сопротивления
    return base * std::max(0.25, derailed_share);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleDerailment::reset()
{
    state = State::OnTrack;
    forced_unload_timer = 0.0;
    rollover_state = RolloverState::None;
    rollover_timer = 0.0;
    surface = SurfaceType::Sleepers;
    std::fill(axle_overload_timer.begin(), axle_overload_timer.end(), 0.0);
    std::fill(derailed_axles.begin(), derailed_axles.end(), false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString VehicleDerailment::getDebugMsg() const
{
    const char* names[] = {"ON TRACK", "WHEEL LIFT", "AXLE DERAILED",
                           "BOGIE DERAILED", "VEHICLE DERAILED"};

    QString msg = QString("Derailment: %1").arg(names[static_cast<int>(state)]);

    for (std::size_t i = 0; i < derailed_axles.size(); ++i)
    {
        msg += QString(" axle%1:%2").arg(i).arg(derailed_axles[i] ? "OFF" : "on");
    }

    return msg;
}
