//------------------------------------------------------------------------------
//
//      Wheel flat spot system (ползун колёсной пары)
//      ТЗ "Реалистичный эффект ползуна на колёсных парах"
//
//      Ползун - реальное повреждение: образуется при юзе (блокировке)
//      колёсной пары, растёт с интенсивностью проскальзывания и нагрузкой,
//      сохраняется после прекращения юза. При каждом обороте колеса
//      ползун проходит зону контакта: колесо получает вертикальный
//      импульс (падение на глубину ползуна), импульс уходит в буксы,
//      тележку, подвеску, кузов (через вертикальную динамику).
//
//      Модель RRS считает одну угловую скорость на колёсную пару,
//      поэтому ползун хранится на ось (оба колеса пары блокируются
//      вместе); задел на по-колёсную детализацию - вектор по осям.
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_FLAT_H
#define     VEHICLE_FLAT_H

#include    <QString>

#include    <cstddef>
#include <vector>

class VehicleVerticalDynamics;

//------------------------------------------------------------------------------
/// Система ползунов колёсных пар единицы ПС
//------------------------------------------------------------------------------
class WheelFlatSystem
{
public:

    VehicleFlatSystem() = default;

    /// Загрузка секции [FlatSpot]
    void loadConfig(QString cfg_path, std::size_t num_axis);

    bool isEnabled() const;

    /// Шаг: углы/скорости колёсных пар, скорость ПЕ, нагрузки осей.
    /// vertical - для передачи вертикального импульса удара в подвеску;
    /// axle_load_kg - осевая нагрузка (масштаб силы удара, эталон 20 т);
    /// ambient_temperature - температура воздуха (охлаждение букс)
    void step(double dt,
              const std::vector<double>& wheel_angles,
              const std::vector<double>& wheel_omegas,
              const std::vector<double>& wheel_radius,
              double velocity,
              VehicleVerticalDynamics& vertical,
              double axle_load_kg = 20000.0,
              double ambient_temperature = 20.0);

    /// Глубина ползуна оси, м
    double getDepth(std::size_t axle) const;

    /// Длина ползуна, м (хорда от глубины и радиуса)
    double getLength(std::size_t axle, double wheel_radius) const;

    /// Сила последнего удара оси, Н (для звука/диагностики)
    double getLastImpactForce(std::size_t axle) const;

    /// Частота ударов оси, Гц (обороты колеса)
    double getImpactRate(std::size_t axle, double velocity,
                         double wheel_radius) const;

    /// Повреждение буксового узла оси (0..1)
    double getBearingDamage(std::size_t axle) const;

    /// Температура буксового узла, град. C
    double getBearingTemperature(std::size_t axle) const;

    /// Полное обточке колеса (ремонт): сброс ползунов
    void reset();

    QString getDebugMsg() const;

private:

    bool enabled = true;

    /// Порог отношения проскальзывания к скорости для образования ползуна
    double slip_threshold = 0.5;

    /// Скорость роста глубины, м/с скольжения при полной нагрузке
    double wear_rate = 0.00004;

    /// Максимальная глубина ползуна, м
    double max_depth = 0.005;

    /// Минимальная скорость ПЕ для образования ползуна, м/с
    double min_speed = 1.0;

    /// Глубина ползуна по осям, м
    std::vector<double> depth;

    /// Угловое положение ползуна на колесе, рад
    std::vector<double> flat_angle;

    /// Сила последнего удара по осям, Н
    std::vector<double> last_impact;

    /// Счётчик ударов по осям
    std::vector<unsigned long> impact_count;

    /// Повреждение букс по осям 0..1
    std::vector<double> bearing_damage;

    /// Температура букс по осям, град. C
    std::vector<double> bearing_temp;

    /// Углы колёсных пар на предыдущем шаге (детектор прохождения
    /// зоны ползуна через точку контакта)
    std::vector<double> prev_angle;

    /// Углы получены хотя бы один раз (первый шаг без детектора)
    bool angles_init = false;

    /// Последняя температура воздуха (сброс при обточке)
    double last_ambient = 20.0;
};

#endif // VEHICLE_FLAT_H
