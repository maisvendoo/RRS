//------------------------------------------------------------------------------
//
//      Simulation LOD manager (уровни детализации симуляции)
//      ТЗ "Оптимизация", п.2-5
//
//      L0 - полный расчёт: поезд игрока, активные составы, участники
//           столкновений. L1 - упрощённый: средняя дистанция (пропуски
//           дорогих подсистем). L2 - агрегированный: дальние составы
//           (только продольная модель). L3 - заморожен: не влияет на
//           игрока (стоящие дальние составы).
//      Уровень назначается по дистанции до поезда игрока и активности;
//      переходы плавные (гистерезис), физика поезда игрока (L0)
//      не упрощается НИКОГДА (критическое правило ТЗ).
//
//------------------------------------------------------------------------------

#ifndef     SIMULATION_LOD_H
#define     SIMULATION_LOD_H

#include    <cstddef>
#include <map>
#include <vector>

namespace perf
{

/// Уровень детализации симуляции
enum class SimLOD : unsigned char
{
    L0_Full = 0,
    L1_Simplified = 1,
    L2_Aggregated = 2,
    L3_Frozen = 3
};

/// Признаки активности поезда (для выбора уровня)
struct TrainActivity
{
    bool is_player_train = false;   ///< Поезд игрока - всегда L0
    bool is_moving = false;         ///< Движется (не L3)
    double distance_to_player = 1e9;///< Метры до поезда игрока
    unsigned model_idx = 0;         ///< Индекс ПЕ (память уровней LOD)
};

//------------------------------------------------------------------------------
/// Менеджер уровней симуляции
//------------------------------------------------------------------------------
class SimulationLODManager
{
public:

    SimulationLODManager() = default;

    /// Радиусы переключения, м (с гистерезисом), и частоты (множители
    /// шага) для L1/L2
    void setDistances(double l1, double l2, double l3);

    /// Назначить уровень поезда по активности. Понижение качества -
    /// сразу при выходе за радиус, повышение - только с запасом
    /// hysteresis_ метров внутри границы (память последних уровней
    /// по model_idx против осцилляций на границе)
    SimLOD classify(const TrainActivity& activity);

    /// Радиус L1 (адаптивный менеджер может менять)
    double getL1Distance() const;
    void setAdaptiveScale(double scale);

private:

    /// Классификация "по расстоянию" без памяти: границы сужаются
    /// на margin метров (margin = hysteresis_ для повышения уровня)
    SimLOD classifyByDistance(double distance, double margin,
                              bool is_moving) const;

    double dist_l1_ = 500.0;
    double dist_l2_ = 2000.0;
    double dist_l3_ = 5000.0;

    /// Гистерезис, м
    double hysteresis_ = 100.0;

    /// Масштаб от adaptive performance (1.0 - норма, меньше - агрессивнее)
    double adaptive_scale_ = 1.0;

    /// Последний назначенный уровень по каждой ПЕ (память гистерезиса)
    std::map<unsigned, SimLOD> last_lod_;
};

} // namespace perf

#endif // SIMULATION_LOD_H
