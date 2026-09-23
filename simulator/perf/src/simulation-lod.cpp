//------------------------------------------------------------------------------
//
//      Simulation LOD manager
//
//------------------------------------------------------------------------------

#include    "simulation-lod.h"

#include    <algorithm>

namespace perf
{

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SimulationLODManager::setDistances(double l1, double l2, double l3)
{
    dist_l1_ = std::max(l1, 50.0);
    dist_l2_ = std::max(l2, dist_l1_ + 100.0);
    dist_l3_ = std::max(l3, dist_l2_ + 500.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SimLOD SimulationLODManager::classify(const TrainActivity& activity)
{
    // Критическое правило (ТЗ, п.24/27): физика поезда игрока не
    // упрощается ниже L0 никогда
    if (activity.is_player_train)
    {
        last_lod_[activity.model_idx] = SimLOD::L0_Full;
        return SimLOD::L0_Full;
    }

    // Уровень "по расстоянию" без памяти (номинальные границы)
    const SimLOD raw = classifyByDistance(activity.distance_to_player, 0.0,
                                          activity.is_moving);

    SimLOD& last = last_lod_[activity.model_idx];

    // Понижение качества (численный уровень вырос) - сразу при
    // выходе за радиус
    if (raw >= last)
    {
        last = raw;
        return raw;
    }

    // Повышение качества - только при запасе hysteresis_ метров
    // внутри границы уровня (границы сужены на гистерезис)
    const SimLOD strict = classifyByDistance(activity.distance_to_player,
                                             hysteresis_,
                                             activity.is_moving);
    if (strict < last)
        last = strict;

    return last;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SimLOD SimulationLODManager::classifyByDistance(double distance, double margin,
                                                bool is_moving) const
{
    if (!is_moving)
    {
        // Стоящий дальний состав без влияния - заморожен
        return (distance > dist_l3_ * adaptive_scale_ - margin)
                ? SimLOD::L3_Frozen
                : SimLOD::L2_Aggregated;
    }

    if (distance < dist_l1_ * adaptive_scale_ - margin)
        return SimLOD::L0_Full;

    if (distance < dist_l2_ * adaptive_scale_ - margin)
        return SimLOD::L1_Simplified;

    return SimLOD::L2_Aggregated;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SimulationLODManager::getL1Distance() const
{
    return dist_l1_ * adaptive_scale_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SimulationLODManager::setAdaptiveScale(double scale)
{
    adaptive_scale_ = std::min(std::max(scale, 0.3), 1.5);
}

} // namespace perf
