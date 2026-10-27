//------------------------------------------------------------------------------
//
//      Wheel wear system (износ бандажей колёсных пар)
//
//------------------------------------------------------------------------------

#include    "vehicle-wheel-wear.h"

#include    <CfgReader.h>

#include    <algorithm>
#include <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WheelWearSystem::loadConfig(QString cfg_path,
                                 std::size_t num_axis_,
                                 double wheel_diameter_m)
{
    num_axis = std::max<std::size_t>(num_axis_, 1);
    wheel_diameter = std::max(wheel_diameter_m, 0.3);

    axle_wear_mm.assign(num_axis, 0.0);
    axle_slip_energy.assign(num_axis, 0.0);
    axle_brake_energy.assign(num_axis, 0.0);
    axle_tonnage.assign(num_axis, 0.0);
    axle_reduction_mm.assign(num_axis, 0.0);

    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "WheelWear";

    bool has_enabled = cfg.getBool(sec, "Enabled", enabled);
    bool any_key = has_enabled;
    any_key = cfg.getDouble(sec, "WearPerTonKm", wear_per_tkm) || any_key;
    cfg.getDouble(sec, "SlipWearPerMJ", slip_wear_per_mj);
    cfg.getDouble(sec, "BrakeWearPerMJ", brake_wear_per_mj);
    cfg.getDouble(sec, "CurvatureCoeff", curvature_coeff);
    cfg.getDouble(sec, "RailConditionCoeff", rail_condition_coeff);
    cfg.getDouble(sec, "WearLimitMm", wear_limit_mm);
    cfg.getDouble(sec, "BaseConicity", base_conicity);
    cfg.getDouble(sec, "ConicityGain", conicity_gain);
    cfg.getDouble(sec, "ThermalCycleTemp", thermal_cycle_temp);
    cfg.getDouble(sec, "ReprofileCutMm", reprofile_cut_mm);
    cfg.getDouble(sec, "ReprofileCutPerWear", reprofile_cut_per_wear);

    // Секция есть: износ включён, если не выключен явно
    if (any_key && !has_enabled)
        enabled = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool WheelWearSystem::isEnabled() const
{
    return enabled;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WheelWearSystem::addAxleSlipEnergy(std::size_t axle, double joules)
{
    if (axle < axle_slip_energy.size() && joules > 0.0)
        axle_slip_energy[axle] += joules;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WheelWearSystem::addAxleBrakeEnergy(std::size_t axle, double joules)
{
    if (axle < axle_brake_energy.size() && joules > 0.0)
        axle_brake_energy[axle] += joules;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WheelWearSystem::step(double dt,
                           double velocity,
                           double full_mass,
                           double curvature,
                           double brake_shoe_max_temp,
                           double rail_condition)
{
    if (!enabled || dt <= 0.0)
        return;

    const double abs_v = std::abs(velocity);

    //--- Пробег и тоннаж (общие на ПЕ) ---

    const double path_km = abs_v * dt / 1000.0;

    mileage_km += path_km;
    tonnage_tkm += full_mass * path_km / 1000.0;

    //--- Циклы нагрева колодок: каждое превышение порога - цикл,
    // ускоряющий усталостный износ поверхности катания

    const bool hot = brake_shoe_max_temp > thermal_cycle_temp;

    if (hot && !was_hot)
        ++thermal_cycles;

    was_hot = hot;

    //--- Факторы эксплуатации (ТЗ "43-47", п.1): параметризуемая
    // формула износа от нагрузки, скорости, крипа, торможений,
    // кривизны и состояния рельса

    const double curve_factor = 1.0 + curvature_coeff *
            std::min(std::abs(curvature), 0.02);

    const double condition_factor = 1.0 + rail_condition_coeff *
            std::min(std::max(rail_condition, 0.0), 1.0);

    // Циклы температуры: умеренный множитель (нарастает с числом циклов,
    // ограничен - старые колёса изнашиваются быстрее, но не бесконечно)
    const double cycles_factor = 1.0 + std::min(0.5,
            0.01 * static_cast<double>(thermal_cycles));

    const double axle_tkm_step = full_mass * path_km / 1000.0 /
            static_cast<double>(num_axis);

    for (std::size_t i = 0; i < num_axis; ++i)
    {
        // База: от тоннажа оси (осевая нагрузка x путь)
        double wear_mm = wear_per_tkm * axle_tkm_step;

        // От энергии проскальзывания (боксование/юз)
        wear_mm += slip_wear_per_mj * axle_slip_energy[i] / 1.0e6;

        // От работы тормозов
        wear_mm += brake_wear_per_mj * axle_brake_energy[i] / 1.0e6;

        wear_mm *= curve_factor * condition_factor * cycles_factor;

        axle_wear_mm[i] += wear_mm;
        axle_tonnage[i] += axle_tkm_step;

        // Аккумуляторы сбрасываются: энергия конвертирована в износ
        axle_slip_energy[i] = 0.0;
        axle_brake_energy[i] = 0.0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelWearSystem::getMileage() const
{
    return mileage_km;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelWearSystem::getTonnage() const
{
    return tonnage_tkm;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelWearSystem::getAxleWearDepth(std::size_t axle) const
{
    if (axle >= axle_wear_mm.size())
        return 0.0;

    return axle_wear_mm[axle];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelWearSystem::getWearDepth() const
{
    if (axle_wear_mm.empty())
        return 0.0;

    double sum = 0.0;
    for (double wear : axle_wear_mm)
        sum += wear;

    return sum / static_cast<double>(axle_wear_mm.size());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelWearSystem::getWear() const
{
    return std::min(std::max(getWearDepth() /
                             std::max(wear_limit_mm, 0.1), 0.0), 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelWearSystem::getConicity() const
{
    // Износ подрезает профиль катания: эффективная коничность растёт,
    // виляние начинается на меньшей скорости (ТЗ, "изменение профиля")
    return std::min(base_conicity + getWear() * conicity_gain, 0.5);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WheelWearSystem::setBaseConicity(double value)
{
    base_conicity = std::min(std::max(value, 0.0), 0.5);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelWearSystem::getAxleTonnage(std::size_t axle) const
{
    if (axle >= axle_tonnage.size())
        return 0.0;

    return axle_tonnage[axle];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WheelWearSystem::reprofile()
{
    // Обточка снимает изношенный слой: профиль (и коничность)
    // восстанавливается, диаметр колеса уменьшается
    for (std::size_t i = 0; i < num_axis; ++i)
    {
        const double cut = reprofile_cut_mm +
                reprofile_cut_per_wear *
                ((i < axle_wear_mm.size()) ? axle_wear_mm[i] : 0.0);

        if (i < axle_reduction_mm.size())
            axle_reduction_mm[i] += cut;

        if (i < axle_wear_mm.size())
            axle_wear_mm[i] = 0.0;

        if (i < axle_tonnage.size())
            axle_tonnage[i] = 0.0;

        if (i < axle_slip_energy.size())
            axle_slip_energy[i] = 0.0;

        if (i < axle_brake_energy.size())
            axle_brake_energy[i] = 0.0;
    }

    ++reprofile_count;

    // Съеденный тоннаж больше не давит на циклы колодок
    thermal_cycles = 0;
    was_hot = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int WheelWearSystem::getReprofileCount() const
{
    return reprofile_count;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelWearSystem::getAxleDiameterReduction(std::size_t axle) const
{
    if (axle >= axle_reduction_mm.size())
        return 0.0;

    return axle_reduction_mm[axle];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double WheelWearSystem::getWheelRadiusFactor(std::size_t axle) const
{
    if (axle >= axle_reduction_mm.size() || wheel_diameter <= 0.0)
        return 1.0;

    // Съём с каждой стороны колеса - половина уменьшения диаметра
    const double diameter_m = wheel_diameter -
            axle_reduction_mm[axle] / 1000.0;

    return std::max(diameter_m / wheel_diameter, 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString WheelWearSystem::getDebugMsg() const
{
    QString msg = QString("WheelWear: %1 km, %2 kt*km, depth %3 mm "
                          "(%4%), conicity %5, turns %6")
            .arg(mileage_km, 0, 'f', 0)
            .arg(tonnage_tkm / 1000.0, 0, 'f', 0)
            .arg(getWearDepth(), 0, 'f', 2)
            .arg(getWear() * 100.0, 0, 'f', 0)
            .arg(getConicity(), 0, 'f', 3)
            .arg(reprofile_count);

    for (std::size_t i = 0; i < axle_wear_mm.size(); ++i)
    {
        msg += QString(" ax%1:%2mm").arg(i).arg(axle_wear_mm[i], 0, 'f', 2);
    }

    return msg;
}
