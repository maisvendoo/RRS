//------------------------------------------------------------------------------
//
//      Track irregularities profile for RRS
//      Rail vertical profile: deterministic irregularities + procedural noise
//
//      Профиль является чистой функцией координаты пути и стороны рельса:
//      z = f(path_coord, rail_side). Никакой зависимости от времени или
//      скорости - скорость прохождения влияет только на динамику отклика ПС.
//      Процедурная генерация детерминирована (хэш координаты + seed),
//      одинакова на всех клиентах и при повторных проездах (ТЗ, п.18, 26).
//
//------------------------------------------------------------------------------

#ifndef     RAIL_PROFILE_H
#define     RAIL_PROFILE_H

#include    "track-profile-export.h"
#include    "track-irregularity.h"
#include    "track-condition.h"

#include    <QString>

#include    <string>
#include    <vector>

namespace track
{

//------------------------------------------------------------------------------
/// Настройки процедурной составляющей профиля (загружаются из конфига маршрута)
//------------------------------------------------------------------------------
struct ProceduralParams
{
    bool enabled = true;
    /// Базовая амплитуда октавы, м
    double amplitude = 0.0012;
    /// Масштаб амплитуды перекоса (разница левого/правого рельса)
    double twist_amplitude = 0.0008;
    /// Амплитуда боковых неровностей плана линии (смещение оси пути), м
    double lateral_amplitude = 0.0025;

    /// Октавы: длины волн, м (короткие ~3 м, средние ~10 м, длинные ~35 м)
    std::vector<double> wavelengths = {3.1, 9.7, 34.0};
    /// Относительные веса октав (младшие - короче - слабее)
    std::vector<double> octave_gains = {0.35, 1.0, 1.6};
    /// Длины волн боковых неровностей плана линии, м
    std::vector<double> lateral_wavelengths = {6.3, 23.0};
};

//------------------------------------------------------------------------------
/// Настройки автоматической генерации стыков и швов
//------------------------------------------------------------------------------
struct JointsParams
{
    /// Шаг изолированных стыков, м (0 - отключены)
    double joint_spacing = 25.0;
    double joint_amplitude = 0.004;
    double joint_length = 0.12;

    /// Шаг сварных швов, м (0 - отключены)
    double weld_spacing = 12.5;
    double weld_amplitude = 0.0015;
    double weld_length = 0.45;
    /// Среднее качество сварки 0..1 (разброс по детерминированной случ.)
    double weld_quality = 0.75;
};

//------------------------------------------------------------------------------
/// Профиль вертикальных неровностей одного пути (траектории).
///
/// Формируется один раз при загрузке маршрута и далее только опрашивается.
/// Хранит отсортированный по координате список неровностей (бинарный поиск
/// по окну) и параметры детерминированного шума.
//------------------------------------------------------------------------------
class TRACKPROFILE_EXPORT RailProfile
{
public:

    RailProfile() = default;

    /// Построить профиль пути: явные неровности + автогенерация стыков/швов +
    /// детерминированный шум. traj_key - имя траектории (участвует в seed)
    void generate(const std::string& traj_key,
                  double traj_length,
                  const std::vector<Irregularity>& explicit_irregularities,
                  const JointsParams& joints,
                  const ProceduralParams& noise,
                  Condition condition,
                  unsigned int seed);

    /// Добавить явную неровность (переупаковывает индекс)
    void addIrregularity(const Irregularity& irregularity);

    /// Вертикальное смещение рельса в точке path_coord, м.
    /// side: 0 - левый, 1 - правый
    double railHeight(double path_coord, int side) const;

    /// Боковое смещение оси пути в точке path_coord, м (план линии).
    /// Детерминировано, возбуждает поперечную динамику ПС (ТЗ
    /// "Поперечная динамика", п.12-13)
    double lateralOffset(double path_coord) const;

    /// Есть ли какая-либо неровность (иначе профиль плоский)
    bool isEmpty() const;

    /// Число явных неровностей
    std::size_t irregularityCount() const;

    /// Повреждение пути (ТЗ "Реалистичный сход ПС", п.27): после схода
    /// участок пути получает повреждения - масштаб неровностей растёт
    /// (влияет на следующие поезда, цепная реакция). Максимум x3
    void degradeTrack(double factor);

    /// Восстановление пути (ремонт)
    void repairTrack();

    /// Текущий множитель повреждения пути (1.0 - целый)
    double getTrackDamageFactor() const;

    /// Накопление тоннажа по участкам (ТЗ "43-47", п.1: износ рельсов).
    /// path_coord - координата прохода; tonnes_per_axle_count - пропущенный
    /// тоннаж, взвешенный по числу осей проходящей ПЕ (т*осей). Износ
    /// накапливается по сетке участков и с насыщением повышает локальный
    /// масштаб неровностей (выработка головки/боковой износ) тем же
    /// паттерном, что и повреждение после схода
    void addTonnage(double path_coord, double tonnes_per_axle_count);

    /// Проход ПЕ массой mass_tonnes т по участку distance_m м через точку
    /// path_coord: добавляет долю тоннажа в участок сетки (тоннаж
    /// распределяется по пройденной доле участка)
    void addPassage(double path_coord, double mass_tonnes, double distance_m);

    /// Локальный фактор тоннажного износа пути в точке (0..MaxWear,
    /// прибавляется к амплитудам неровностей как доля)
    double tonnageWear(double path_coord) const;

    /// Накопленный тоннаж в точке, т*осей
    double getTonnage(double path_coord) const;

    /// Предел тоннажного износа (доля от амплитуд неровностей)
    double getTonnageWearLimit() const;

private:

    /// Детерминированный шум для стороны side, м
    double noise(double path_coord, int side) const;

    /// Детерминированный шум бокового смещения оси пути, м
    double lateralNoise(double path_coord) const;

    /// Сумма явных неровностей в точке для стороны side, м
    double explicitSum(double path_coord, int side) const;

    void rebuildIndex();

    std::vector<Irregularity> irregularities_;

    /// Индекс: неровности, отсортированные по началу зоны воздействия
    std::vector<const Irregularity*> sorted_by_begin_;

    /// Максимальная длина зоны воздействия (для окна поиска)
    double max_length_ = 0.0;

    ProceduralParams noise_params_;
    /// Масштаб шума по состоянию пути
    double condition_scale_ = 1.0;

    /// Seed шума, смешанный с именем пути и стороной рельса
    unsigned int noise_seed_ = 0;

    /// Множитель повреждения пути после аварий (1.0 - целый)
    double track_damage_factor_ = 1.0;

    /// Участок накопленного тоннажа (износ рельсов от проходов)
    struct TonnageBin
    {
        double coord = 0.0;     ///< Центр участка, м
        double tonnes = 0.0;    ///< Накопленный тоннаж, т*осей
    };

    /// Сетка накопления тоннажа (отсортирована по координате)
    std::vector<TonnageBin> tonnage_bins_;

    /// Шаг сетки накопления, м
    double tonnage_bin_size_ = 25.0;

    /// Прирост износа на тонну*осей (доля амплитуды)
    double tonnage_wear_rate_ = 2.0e-7;

    /// Насыщение тоннажного износа (максимум +60% к амплитудам)
    double tonnage_wear_limit_ = 0.6;
};

//------------------------------------------------------------------------------
/// Зона возвышения наружного рельса (секция [Cant] конфига маршрута).
/// Возвышение - свойство ПУТИ (Б16): внутри [Begin, End] держится CantMm,
/// между соседними треками значения интерполируются линейно (отвод
/// возвышения). Знак: "+" - левый рельс выше, "-" - правый
//------------------------------------------------------------------------------
struct TRACKPROFILE_EXPORT CantZone
{
    /// Имя траектории ("" или "*" - применяется ко всем путям маршрута)
    QString traj_name = "";
    double begin = 0.0;     ///< Начало зоны, м (координата траектории)
    double end = 0.0;       ///< Конец зоны, м
    double cant_mm = 0.0;   ///< Возвышение наружного рельса, мм
};

//------------------------------------------------------------------------------
/// Конфигурация профилей пути маршрута (файл track-profile.conf в каталоге
/// маршрута). Кэшируется: файл читается один раз на все траектории
//------------------------------------------------------------------------------
struct TRACKPROFILE_EXPORT TrackProfileConfig
{
    bool enabled = true;
    unsigned int seed = 18273;
    Condition condition = Condition::Normal;
    JointsParams joints;
    ProceduralParams noise;

    /// Автогенерация неровностей на стрелочных переводах (крестовина:
    /// двойной импульс + жёсткий удар, ТЗ "Неровности", п.15)
    bool switch_irregularity = true;
    double switch_amplitude = 0.006;    ///< Базовая амплитуда зоны, м
    double switch_length = 6.0;         ///< Длина зоны перевода, м

    /// Явные неровности, применяемые ко всем траекториям маршрута
    std::vector<Irregularity> explicit_irregularities;

    /// Явные неровности, привязанные к конкретной траектории
    /// (имя траектории, неровность)
    std::vector<std::pair<QString, Irregularity>> named_irregularities;

    /// Состояние пути для конкретной траектории (пусто - общий уровень)
    std::vector<std::pair<QString, Condition>> traj_conditions;

    /// Зоны возвышения наружного рельса (Б16, "Поперечная динамика" п.14-15)
    std::vector<CantZone> cants;
};

/// Прочитать конфиг маршрута. Отсутствие файла - не ошибка (умолчания),
/// возвращается false только при ошибке чтения существующего файла
TRACKPROFILE_EXPORT bool loadTrackProfileConfig(const QString& route_dir,
                                                TrackProfileConfig& out,
                                                QString* error = nullptr);

} // namespace track

#endif // RAIL_PROFILE_H
