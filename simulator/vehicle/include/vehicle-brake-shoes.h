//------------------------------------------------------------------------------
//
//      Brake shoes system (нагрев, износ и fade тормозных колодок)
//      ТЗ "Реалистичная система нагрева, износа и потери эффективности
//      тормозных колодок"
//
//      Колодка - физический элемент: нагрев от реальной работы тормоза
//      (момент × скорость колеса), охлаждение потоком воздуха, fade
//      (падение трения с температурой), износ толщины от энергии и
//      температуры. Неравномерность: две колодки на ось со своими
//      вариациями. Эффективность влияет на тормозную силу осей.
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_BRAKE_SHOES_H
#define     VEHICLE_BRAKE_SHOES_H

#include    <QString>

#include    <cstddef>
#include <vector>

//------------------------------------------------------------------------------
/// Система тормозных колодок единицы ПС
//------------------------------------------------------------------------------
class BrakeShoeSystem
{
public:

    /// Материал колодки (ТЗ, п.11)
    enum class ShoeType
    {
        CastIron = 0,   ///< Чугунные
        Composite = 1,  ///< Композитные
        LowFriction = 2,///< Малой фрикции
        HighFriction = 3,///< Повышенной фрикции
        Custom = 4
    };

    BrakeShoeSystem() = default;

    /// Загрузка секции [BrakeShoes]
    void loadConfig(QString cfg_path, std::size_t num_axis);

    bool isEnabled() const;

    /// Шаг: момент тормоза на осях (Н*м), скорости колёс (рад/с),
    /// скорость ПЕ (охлаждение), температура воздуха
    void step(double dt,
              const std::vector<double>& brake_torques,
              const std::vector<double>& wheel_omegas,
              double velocity,
              double air_temperature);

    /// Эффективность торможения оси (0..1, fade + износ)
    double getAxleEfficiency(std::size_t axle) const;

    /// Средняя эффективность (диагностика)
    double getAverageEfficiency() const;

    /// Максимальная температура колодок, град. C
    double getMaxTemperature() const;

    /// Температура колодки оси/стороны (side 0/1), град. C
    double getTemperature(std::size_t axle, int side) const;

    /// Остаток толщины колодки оси/стороны (0..1)
    double getThickness(std::size_t axle, int side) const;

    /// Есть изношенные до предела колодки
    bool hasWornOut() const;

    /// Замена колодок (обслуживание)
    void replaceShoes();

    QString getDebugMsg() const;

private:

    struct Pad
    {
        double temperature = 20.0;  ///< град. C
        double thickness = 1.0;     ///< доля от новой
        double variation = 1.0;     ///< локальная вариация износа
        double efficiency = 1.0;
    };

    /// Fade: множитель трения от температуры (ТЗ, п.5)
    double fadeFactor(double temperature) const;

    bool enabled = true;

    ShoeType type = ShoeType::Composite;

    /// Границы температур fade, град. C
    double t_normal = 150.0;
    double t_fade = 350.0;
    double t_critical = 600.0;

    /// Эффективность на границах (доли)
    double eff_at_fade = 0.6;
    double eff_at_critical = 0.25;
    double eff_over_critical = 0.1;

    /// Теплоёмкость колодки+колеса, Дж/К
    double thermal_capacity = 20000.0;

    /// Коэффициент охлаждения, Вт/К при 20 м/с
    double cooling_coeff = 60.0;

    /// Доля тормозной работы, идущая в колодку (остальное - колесо)
    double heat_to_shoe = 0.5;

    /// Скорость износа: мм на МДж работы при 100 град. C
    double wear_per_mj = 0.4;
    double wear_temp_factor = 2.0;   ///< рост износа при крит. температуре

    /// Толщина новой колодки, мм (износ в долях от неё)
    double max_thickness_mm = 40.0;

    /// Базовый коэффициент трения материала (множитель к эффективности)
    double friction_coeff = 1.0;

    /// Минимальная безопасная толщина (доля)
    double min_thickness = 0.15;

    /// Колодки: ось x сторона
    std::vector<Pad> pads;

    /// Последняя температура воздуха (сброс температуры при замене)
    double last_ambient = 20.0;

    /// Предупреждения журнала (однократно на эпизод, на экземпляр)
    bool warned_hot = false;
    bool warned_worn = false;
};

#endif // VEHICLE_BRAKE_SHOES_H
