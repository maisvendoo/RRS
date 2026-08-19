//------------------------------------------------------------------------------
//
//      Vehicle damage system (компонентные повреждения 0..1)
//      ТЗ "Реалистичный сход ПС", п.15-17
//
//      Повреждение - не бинарь: каждый компонент 0.0 (исправно) ..
//      1.0 (разрушено). Повреждение зависит от энергии воздействия,
//      его места и типа конструкции: небольшой контакт не уничтожает ПЕ.
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_DAMAGE_H
#define     VEHICLE_DAMAGE_H

#include    <QString>

#include    <cstddef>
#include    <vector>

//------------------------------------------------------------------------------
/// Компонентные повреждения единицы подвижного состава
//------------------------------------------------------------------------------
class VehicleDamageSystem
{
public:

    /// Компоненты повреждений (ТЗ, п.15)
    enum class Component
    {
        Coupler = 0,    ///< Сцепные устройства
        Bogie = 1,      ///< Ходовая часть (тележки)
        Wheel = 2,      ///< Колёсные пары
        Brake = 3,      ///< Тормозное оборудование
        Body = 4,       ///< Кузов/рама
        Electrical = 5, ///< Электрооборудование
        Engine = 6,     ///< Силовая установка
        Tank = 7,       ///< Ёмкости (топливные баки, котлы цистерн)

        Count
    };

    /// Зона удара для распределения повреждений (ТЗ, п.17: место контакта)
    enum class ImpactZone
    {
        Front = 0,  ///< Удар в переднюю часть (сцепка, рама)
        Rear = 1,   ///< Удар в заднюю часть
        Side = 2,   ///< Боковой удар (кузов, тележки)
        Bottom = 3, ///< Удар снизу (сход, путь)
        Top = 4     ///< Падение сверху / переворот
    };

    VehicleDamageSystem() = default;

    /// Загрузка секции [DamageSystem] (пороги энергии разрушения, Дж)
    void loadConfig(QString cfg_path);

    /// Повреждение от столкновения: кинетическая энергия удара по зоне.
    /// Энергия ниже порога не даёт заметных повреждений (п.17)
    void applyImpact(double energy, ImpactZone zone);

    /// Накопление повреждений от движения в сошедшем состоянии
    /// (движение по шпалам разрушает ходовую, кузов, тормоза)
    void applyDerailmentWear(double dt, double velocity);

    /// Повреждение компонента напрямую (внешние системы)
    void addDamage(Component component, double value);

    /// Повреждение компонента 0..1
    double getDamage(Component component) const;

    /// Доля исправности тормозной системы 1..0 (для тормозной силы)
    double getBrakeEfficiency() const;

    /// Доля исправности сцепных устройств
    double getCouplerStrengthFactor() const;

    /// Полностью разрушена (кузов или ходовая)
    bool isDestroyed() const;

    /// Ремонт (полный сброс)
    void reset();

    QString getDebugMsg() const;

private:

    double damage[static_cast<std::size_t>(Component::Count)] = {};

    /// Энергия, разрушающая компонент на 100%, Дж (по зонам удара)
    double body_break_energy = 8.0e6;
    double bogie_break_energy = 3.0e6;
    double coupler_break_energy = 2.0e6;
    double brake_break_energy = 4.0e6;
    double tank_break_energy = 5.0e6;

    /// Порог энергии, ниже которого повреждений нет, Дж
    double impact_threshold = 30e3;

    /// Скорость накопления повреждений при движении после схода (1/с при 1 м/с)
    double derail_wear_rate = 0.01;
};

#endif // VEHICLE_DAMAGE_H
