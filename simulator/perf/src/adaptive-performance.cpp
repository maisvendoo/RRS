//------------------------------------------------------------------------------
//
//      Adaptive performance manager
//
//------------------------------------------------------------------------------

#include    "adaptive-performance.h"
#include    "perf-profiler.h"

#include    <algorithm>

namespace perf
{

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AdaptivePerformanceManager::setTargetPhysicsMs(double ms)
{
    target_ms_ = std::max(ms, 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AdaptivePerformanceManager::step(double dt, const Profiler& profiler,
                                      SimulationLODManager& lod)
{
    const double frame_ms = profiler.getFrameMs();

    if (frame_ms <= 0.0)
        return;

    // Перегрузка - сжимаем радиусы LOD (с запасом от осцилляций)
    if (frame_ms > target_ms_ * (1.0 + margin_))
    {
        scale_ -= rate_ * dt;
    }
    // Запас большой - плавно возвращаем качество
    else if (frame_ms < target_ms_ * (1.0 - margin_))
    {
        scale_ += rate_ * dt * 0.5;
    }

    scale_ = std::min(std::max(scale_, 0.3), 1.5);

    lod.setAdaptiveScale(scale_);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double AdaptivePerformanceManager::getScale() const
{
    return scale_;
}

} // namespace perf
