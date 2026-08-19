//------------------------------------------------------------------------------
//
//      Derailment system (постепенный сход с рельсов)
//      ТЗ "Динамика ПС" (ProdVertKoleb) п.11-12, основа для ТЗ "Реалистичный
//      сход ПС" и "Физика после схода"
//
//      Сход - не случайное событие и не мгновенная телепортация: машина
//      состояний, управляемая физическими датчиками поперечной и
//      вертикальной динамики:
//      1. рост боковых сил (Y/Q приближается к пределу Надаля) - предупреждение;
//      2. разгрузка колеса (вертикальная динамика) + превышение Y/Q
//         устойчивое время - подъём гребня (WheelLift);
//      3. сход колёсной пары (AxleDerailed);
//      4. обе оси тележки - сход тележки (BogieDerailed);
//      5. обе тележки - сход ПЕ (VehicleDerailed).
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_DERAILMENT_H
#define     VEHICLE_DERAILMENT_H

#include    <QString>

#include    <cstddef>
#include    <vector>

class VehicleLateralDynamics;
class VehicleVerticalDynamics;

//------------------------------------------------------------------------------
/// Система схода единицы подвижного состава
//------------------------------------------------------------------------------
class VehicleDerailment
{
public:

    /// Стадии схода
    enum class State
    {
        OnTrack = 0,        ///< На рельсах
        WheelLiftWarning = 1,///< Подъём гребня: Y/Q у предела, колесо разгружено
        AxleDerailed = 2,   ///< Сошла хотя бы одна колёсная пара
        BogieDerailed = 3,  ///< Сошла тележка (все её оси)
        VehicleDerailed = 4 ///< Сошла вся ПЕ (все тележки)
    };

    /// Состояние опрокидывания (ТЗ "Физика после схода", п.10):
    /// физический критерий - боковое ускорение против плеча центра масс
    enum class RolloverState
    {
        None = 0,           ///< Устойчиво
        Warning = 1,        ///< Приближение к пределу (запас < 15%)
        Partial = 2,        ///< Частичное опрокидывание (на гребне/упоре)
        Full = 3            ///< Полное опрокидывание
    };

    /// Тип поверхности после схода (п.3-5): сопротивление различается
    enum class SurfaceType
    {
        Sleepers = 0,       ///< Шпалы: сильные удары, большое сопротивление
        Ballast = 1,        ///< Щебень: большое сопротивление качению
        Ground = 2          ///< Грунт: проваливание, максимальное сопротивление
    };

    VehicleDerailment() = default;

    /// Загрузка из секции [Derailment] конфига ПЕ
    void loadConfig(QString cfg_path, std::size_t num_axis, std::size_t vehicle_idx = 0);

    bool isEnabled() const;

    /// Шаг оценки схода по датчикам динамики
    void step(double dt,
              const VehicleLateralDynamics& lateral,
              const VehicleVerticalDynamics& vertical,
              std::size_t num_axis);

    /// Шаг опрокидывания и контакта с полотном после схода.
    /// velocity - скорость ПЕ, м/с; lateral_accel - боковое ускорение
    /// кузова (включая кривую), м/с^2
    void stepRollover(double dt, double velocity, double lateral_accel);

    /// Высота центра масс, м (для критерия опрокидывания)
    void setMassCenterHeight(double height);

    /// Текущая стадия
    State getState() const;

    /// Состояние опрокидывания
    RolloverState getRolloverState() const;

    /// Полное опрокидывание
    bool isRollover() const;

    /// Тип поверхности движения после схода
    SurfaceType getSurfaceType() const;
    void setSurfaceType(SurfaceType surface);

    /// Частота ударов о шпалы, Гц (для звука/эффектов: скорость / шаг шпал)
    double getSleeperImpactRate() const;

    /// ПЕ сошла с рельсов (хотя бы одна ось)
    bool isDerailed() const;

    /// Сошла вся ПЕ
    bool isVehicleDerailed() const;

    /// Индексы сошедших осей
    const std::vector<bool>& getDerailedAxles() const;

    /// Сброс (восстановление после аварии)
    void reset();

    /// Боковой удар (столкновение): импульс бросает кузов, колёса с
    /// стороны удара разгружаются (ТЗ "Реалистичный сход ПС", п.2, 14).
    /// energy - полная энергия удара, Дж; нормаль и касательная пути -
    /// для выделения боковой составляющей
    void applyLateralImpact(double energy,
                            double nx, double ny, double nz,
                            double ox, double oy, double oz,
                            double o_len);

    /// Дополнительное удельное сопротивление после схода (доля от веса)
    double getResistanceCoefficient() const;

    QString getDebugMsg() const;

private:

    bool enabled = true;

    /// Предел Y/Q (0 - использовать критерий Надаля из поперечной динамики)
    double yq_limit_override = 0.0;

    /// Доля статической нагрузки, ниже которой колесо считается
    /// разгруженным (входит в условие подъёма гребня)
    double wheel_unload_limit = 0.2;

    /// Время устойчивого превышения Y/Q до подъёма гребня, с
    double flange_climb_time = 0.05;

    /// Минимальное боковое смещение оси для схода (колесо на гребне), м
    double derail_displacement = 0.02;

    /// Сопротивление движению после схода (доля от веса)
    double derailed_resistance = 0.15;

    /// Индекс ПЕ для журнала событий
    std::size_t vehicle_idx = 0;

    /// Таймер принудительной разгрузки колёс после бокового удара, с
    double forced_unload_timer = 0.0;

    /// Порог боковой энергии удара для разгрузки колёс, Дж
    double lateral_impact_threshold = 150e3;

    /// Высота центра масс над УГР, м
    double mass_center_height = 1.8;

    /// Полуширина колеи опрокидывания (плечо), м
    double rollover_arm = 0.77;

    /// Состояние опрокидывания
    RolloverState rollover_state = RolloverState::None;

    /// Время устойчивого превышения предела опрокидывания, с
    double rollover_timer = 0.0;

    /// Тип поверхности после схода
    SurfaceType surface = SurfaceType::Sleepers;

    /// Сопротивление по поверхностям (доля от веса, на всю ПЕ)
    double resistance_sleepers = 0.30;
    double resistance_ballast = 0.18;
    double resistance_ground = 0.40;

    /// Шаг шпал для частоты ударов, м
    double sleeper_spacing = 0.25;

    /// Скорость для расчёта частоты ударов о шпалы
    double last_velocity = 0.0;

    State state = State::OnTrack;

    /// Таймеры устойчивого превышения Y/Q на осях, с
    std::vector<double> axle_overload_timer;

    /// Сошедшие оси
    std::vector<bool> derailed_axles;
};

#endif // VEHICLE_DERAILMENT_H
