//------------------------------------------------------------------------------
//
//      Hazard cargo system (опасные грузы, утечки, пожар, взрыв)
//      ТЗ "Реалистичный сход ПС", п.22-26, 38
//
//      Игровая абстрактная модель последствий (не детальная химия):
//      состояние ёмкости Normal -> Damaged -> Leak -> Critical;
//      пожар (интенсивность/распространение), взрыв как следствие
//      утечки горючего + источника воспламенения. Цепные реакции
//      ограничены глубиной и частотой (защита от физического взрыва
//      системы, п.38).
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_HAZARD_H
#define     VEHICLE_HAZARD_H

#include    <QString>

#include    <cstddef>

#ifndef VEHICLE_EXPORT
    #if defined(VEHICLE_LIB)
        #define VEHICLE_EXPORT Q_DECL_EXPORT
    #else
        #define VEHICLE_EXPORT Q_DECL_IMPORT
    #endif
#endif

//------------------------------------------------------------------------------
/// Опасный груз единицы подвижного состава
//------------------------------------------------------------------------------
class VEHICLE_EXPORT VehicleHazard
{
public:

    /// Категория груза (ТЗ, п.22)
    enum class CargoType
    {
        Normal = 0,     ///< Обычный груз/пассажиры
        Flammable = 1,  ///< Горючее (топливо, лес и т.п.)
        Explosive = 2,  ///< Взрывоопасный
        Toxic = 3,      ///< Токсичный
        Corrosive = 4,  ///< Едкий
        Pressurized = 5 ///< Сжиженный газ под давлением
    };

    /// Состояние ёмкости с грузом (ТЗ, п.23)
    enum class TankState
    {
        Normal = 0,
        Damaged = 1,
        Leak = 2,
        Critical = 3
    };

    VehicleHazard() = default;

    /// Загрузка секции [Cargo]
    void loadConfig(QString cfg_path);

    /// Шаг системы.
    /// tank_damage - повреждение ёмкости из системы повреждений (0..1);
    /// body_damage - повреждение кузова; fire_nearby - пожар рядом
    /// (цепная реакция); dt - шаг, с
    void step(double dt,
              double tank_damage,
              double body_damage,
              bool fire_nearby);

    CargoType getCargoType() const;
    TankState getTankState() const;

    /// Горит
    bool isOnFire() const;

    /// Интенсивность пожара 0..1
    double getFireIntensity() const;

    /// Взрыв произошёл на последнем шаге (однократное событие)
    bool consumeExplosionEvent();

    /// Радиус воздействия взрыва/пожара для соседних ПЕ, м
    double getHazardRadius() const;

    /// Токсичная/горящая зона препятствует движению
    bool isZoneDangerous() const;

    /// Полный сброс
    void reset();

    QString getDebugMsg() const;

private:

    void tryIgnite(bool fire_nearby);

    CargoType cargo = CargoType::Normal;

    TankState tank_state = TankState::Normal;

    bool on_fire = false;
    double fire_intensity = 0.0;

    bool explosion_event = false;

    /// Порог повреждения ёмкости для утечки
    double leak_damage_threshold = 0.5;

    /// Вероятность возгорания при утечке горючего, 1/с
    double ignite_rate = 0.02;

    /// Скорость роста пожара, 1/с
    double fire_growth = 0.15;

    /// Порог состояния ёмкости для взрыва (для explosive/pressurized)
    double explosion_threshold = 0.9;

    /// Время существования пожара до затухания/выгорания, с
    double fire_burnout_time = 300.0;

    double fire_time = 0.0;

    /// Защита от цепного взрыва: после взрыва повтор невозможен
    bool exploded = false;
};

#endif // VEHICLE_HAZARD_H
