//------------------------------------------------------------------------------
//
//      Adaptive performance manager (динамическая оптимизация)
//      ТЗ "Оптимизация", п.24
//
//      Анализирует время кадра/физики и автоматически управляет
//      масштабом LOD симуляции. КРИТИЧЕСКАЯ ФИЗИКА ПОЕЗДА ИГРОКА НЕ
//      УПРОЩАЕТСЯ НИЖЕ L0 (встроено в SimulationLODManager).
//
//------------------------------------------------------------------------------

#ifndef     ADAPTIVE_PERFORMANCE_H
#define     ADAPTIVE_PERFORMANCE_H

#include    "simulation-lod.h"

#include    <cstddef>

namespace perf
{

class Profiler;

//------------------------------------------------------------------------------
/// Менеджер адаптивной производительности
//------------------------------------------------------------------------------
class AdaptivePerformanceManager
{
public:

    AdaptivePerformanceManager() = default;

    /// Целевое время физики, мс/кадр (бюджет)
    void setTargetPhysicsMs(double ms);

    /// Шаг: смотрит профиль и двигает масштаб LOD (плавно)
    void step(double dt, const Profiler& profiler,
              SimulationLODManager& lod);

    /// Текущий масштаб LOD (0.3..1.5)
    double getScale() const;

private:

    double target_ms_ = 8.0;

    double scale_ = 1.0;

    /// Плавность изменений, 1/с
    double rate_ = 0.2;

    /// Анти-осцилляция: гистерезис в долях цели
    double margin_ = 0.15;
};

} // namespace perf

#endif // ADAPTIVE_PERFORMANCE_H
