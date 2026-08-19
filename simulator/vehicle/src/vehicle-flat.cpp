//------------------------------------------------------------------------------
//
//      Wheel flat spot system (ползун колёсной пары)
//
//------------------------------------------------------------------------------

#include    "vehicle-flat.h"

#include    "vehicle-dynamics.h"

#include    "physics.h"

#include    <CfgReader.h>

#include    <algorithm>
#include    <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
WheelFlatSystem::WheelFlatSystem() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WheelFlatSystem::loadConfig(QString cfg_path, std::size_t num_axis)
{
    depth.assign(num_axis, 0.0);
    flat_angle.assign(num_axis, 0.0);
    last_impact.assign(num_axis, 0.0);
    impact_count.assign(num_axis, 0);
    bearing_damage.assign(num_axis, 0.0);
    bearing_temp.assign(num_axis, 20.0);
    prev_angle.assign(num_axis, 0.0);
    angles_init = false;

    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "FlatSpot";

    cfg.getBool(sec, "Enabled", enabled);
    cfg.getDouble(sec, "SlipThreshold", slip_threshold);
    cfg.getDouble(sec, "WearRate", wear_rate);
    cfg.getDouble(sec, "MaxDepth", max_depth);
    cfg.getDouble(sec, "MinSpeed", min_speed);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool WheelFlatSystem::isEnabled() const
{
    return enabled;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WheelFlatSystem::step(double dt,
                           const std::vector<double>& wheel_angles,
                           const std::vector<double>& wheel_omegas,
                           const std::vector<double>& wheel_radius,
                           double velocity,
                           VehicleVerticalDynamics& vertical,
                           double axle_load_kg,
                           double ambient_temperature)
{
    if (!enabled)
        return;

    const double abs_v = std::abs(velocity);
    const double dir = (velocity >= 0.0) ? 1.0 : -1.0;
    const double TWO_PI = 2.0 * Physics::PI;

    last_ambient = ambient_temperature;

    // Первый шаг: запоминаем углы, детектор ударов включаем со второго
    // (иначе переход 0 -> фактический угол даст ложный удар)
    if (!angles_init)
    {
        angles_init = true;
        for (std::size_t i = 0; i < prev_angle.size(); ++i)
            prev_angle[i] = (i < wheel_angles.size()) ? wheel_angles[i] : 0.0;
    }

    for (std::size_t i = 0; i < depth.size(); ++i)
    {
        const double r = (i < wheel_radius.size()) ? wheel_radius[i] : 0.475;
        const double omega = (i < wheel_omegas.size()) ? wheel_omegas[i] : 0.0;
        const double angle = (i < wheel_angles.size()) ? wheel_angles[i] : 0.0;

        //--- 1. Образование ползуна при юзе (ТЗ, п.1-3) ---

        if (abs_v > min_speed && r > 0.05)
        {
            // Линейная скорость обода против скорости ПЕ. Ползун
            // образуется только при ЮЗЕ - колесо медленнее поезда;
            // боксование (колесо быстрее поезда) юзом не считается
            const double rim_speed = omega * r;
            const double skid = (velocity - rim_speed) * dir;

            if (skid > slip_threshold * abs_v)
            {
                // Рост глубины: интенсивность скольжения (относительная
                // скорость юза) на нагрузке. Ползун остаётся после
                // прекращения юза (сохраняется в depth)
                const double growth = wear_rate * skid * dt;

                const double prev_depth = depth[i];
                depth[i] = std::min(max_depth, depth[i] + growth);

                if (prev_depth <= 0.0 && depth[i] > 0.0)
                {
                    // Ползун образуется в текущем угловом положении
                    flat_angle[i] = angle;
                }
            }
        }

        //--- 2. Удар при прохождении ползуна через точку контакта ---
        // Зона повреждения жёстко связана с колесом: удар происходит,
        // когда угол колеса проходит положение ползуна (частота -
        // обороты колеса). Заблокированное колесо (омега ~ 0) зону
        // через контакт не проводит - ударов нет

        const double rim_speed = std::abs(omega) * r;

        if (depth[i] > 1e-5 && rim_speed > 0.05 && r > 0.05)
        {
            // Фаза ползуна на колесе, нормированная в [0, 2*pi)
            auto flat_phase = [&](double a) -> double
            {
                double d = std::fmod(a - flat_angle[i], TWO_PI);
                if (d < 0.0)
                    d += TWO_PI;
                return d;
            };

            const double d_now = flat_phase(angle);
            const double d_prev = flat_phase(prev_angle[i]);

            // Переход фазы через 0 (вперёд или назад) = зона ползуна
            // прошла точку контакта
            const bool crossed = (d_prev - d_now > Physics::PI) ||
                                 (d_now - d_prev > Physics::PI);

            if (crossed)
            {
                // Скорость падения на глубину ползуна: v = sqrt(2*g*h),
                // далее контакт Герца отбивает. Глубже ползун - сильнее
                // удар (п.8), сила растёт и со скорости (энергия удара)
                const double drop_speed = std::sqrt(2.0 * Physics::g * depth[i]);

                // Масштаб от осевой нагрузки (эталон 20 т/ось)
                const double load_factor =
                        std::max(axle_load_kg, 1000.0) / 20000.0;

                // Динамическая сила удара: колёсная пара (~1.4 т)
                // останавливается контактом за ~2 мс; на скорости круг
                // "втыкается" в лыску с усилием растущим от скорости
                const double impact_force = 1400.0 * drop_speed / 0.002 *
                        std::min(abs_v / 10.0 + 0.5, 3.0) * load_factor;

                last_impact[i] = impact_force;
                ++impact_count[i];

                // Вертикальный импульс в подвеску: колесо "падает" в лыску
                vertical.applyFlatImpact(i, std::min(drop_speed, 0.4));

                //--- Нагрузка на буксы (ТЗ, п.9-11) ---

                // Износ буксы от ударной перегрузки
                const double overload = impact_force / 300e3;

                if (overload > 1.0)
                {
                    bearing_damage[i] = std::min(1.0, bearing_damage[i] +
                                                 0.0005 * overload);
                }

                // Нагрев буксы пропорционален силе удара. Откалибровано
                // на длительное движение с ползуном 2+ мм при 20 т/ось:
                // равновесная температура выходит на 90+ C - порог
                // ускоренного износа достижим
                bearing_temp[i] += 1.5e-7 * impact_force;

                if (bearing_temp[i] > 90.0)
                {
                    // Перегрев буксы ускоряет износ
                    bearing_damage[i] = std::min(1.0, bearing_damage[i] + 0.0001);
                }
            }
        }

        prev_angle[i] = angle;

        // Охлаждение букс к воздуху (медленное, инерционное)
        bearing_temp[i] += (ambient_temperature - bearing_temp[i]) *
                std::min(1.0, 0.005 * dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelFlatSystem::getDepth(std::size_t axle) const
{
    if (axle >= depth.size())
        return 0.0;

    return depth[axle];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelFlatSystem::getLength(std::size_t axle, double wheel_radius) const
{
    const double d = getDepth(axle);

    if (d <= 0.0 || wheel_radius < d)
        return 0.0;

    // Хорда кругового сегмента: L = 2*sqrt(2*R*d - d^2)
    return 2.0 * std::sqrt(std::max(2.0 * wheel_radius * d - d * d, 0.0));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelFlatSystem::getLastImpactForce(std::size_t axle) const
{
    if (axle >= last_impact.size())
        return 0.0;

    return last_impact[axle];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelFlatSystem::getImpactRate(std::size_t axle, double velocity,
                                      double wheel_radius) const
{
    if (axle >= depth.size() || depth[axle] <= 0.0 || wheel_radius < 0.05)
        return 0.0;

    const double circumference = 2.0 * Physics::PI * wheel_radius;

    return std::abs(velocity) / circumference;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelFlatSystem::getBearingDamage(std::size_t axle) const
{
    if (axle >= bearing_damage.size())
        return 0.0;

    return bearing_damage[axle];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelFlatSystem::getBearingTemperature(std::size_t axle) const
{
    if (axle >= bearing_temp.size())
        return 20.0;

    return bearing_temp[axle];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WheelFlatSystem::reset()
{
    std::fill(depth.begin(), depth.end(), 0.0);
    std::fill(last_impact.begin(), last_impact.end(), 0.0);
    std::fill(impact_count.begin(), impact_count.end(), 0);
    std::fill(bearing_damage.begin(), bearing_damage.end(), 0.0);
    std::fill(bearing_temp.begin(), bearing_temp.end(), last_ambient);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString WheelFlatSystem::getDebugMsg() const
{
    QString msg = "Flat spots:";

    for (std::size_t i = 0; i < depth.size(); ++i)
    {
        msg += QString(" axle%1: %2 mm (%3 imp, bear %4%)")
                .arg(i)
                .arg(depth[i] * 1000.0, 0, 'f', 1)
                .arg(impact_count[i])
                .arg(bearing_damage[i] * 100.0, 0, 'f', 1);
    }

    return msg;
}
