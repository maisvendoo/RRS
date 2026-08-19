//------------------------------------------------------------------------------
//
//      Track irregularities profile for RRS
//      Rail profile: deterministic noise, joints generation, config load
//
//------------------------------------------------------------------------------

#include    "rail-profile.h"

#include    <CfgReader.h>

#include    <algorithm>
#include    <cmath>
#include    <cstring>
#include    <mutex>

namespace track
{

namespace
{

constexpr double pi = 3.14159265358979323846;

//------------------------------------------------------------------------------
/// Детерминированное смешивание бит (MurmurHash3 finalizer).
/// Одинаковые входные данные дают одинаковый результат на любой платформе -
/// без использования float-округлений и стандартного random()
//------------------------------------------------------------------------------
std::uint32_t mix32(std::uint32_t value)
{
    value ^= value >> 16;
    value *= 0x85ebca6bu;
    value ^= value >> 13;
    value *= 0xc2b2ae35u;
    value ^= value >> 16;
    return value;
}

/// Хэш строки (seed перемешивается каждым символом)
std::uint32_t hashString(const std::string& text, std::uint32_t seed)
{
    for (char ch : text)
    {
        seed = mix32(seed ^ static_cast<std::uint8_t>(ch));
    }
    return seed;
}

/// Детерминированное число в [0, 1) по целочисленной решётке координат
double latticeNoise(std::uint32_t ix, std::uint32_t channel)
{
    const std::uint32_t bits = mix32(mix32(ix) ^ (channel * 0x9e3779b9u));
    return static_cast<double>(bits) / 4294967296.0;
}

/// Косинусная интерполяция узлов решётки: C1-непрерывный шум по координате
double valueNoise(double coord, double wavelength, std::uint32_t channel)
{
    const double t = coord / std::max(wavelength, 0.1);
    const double floor_t = std::floor(t);
    const double frac = t - floor_t;

    const auto i0 = static_cast<std::uint32_t>(static_cast<std::int64_t>(floor_t));
    const std::uint32_t i1 = i0 + 1u;

    const double n0 = latticeNoise(i0, channel);
    const double n1 = latticeNoise(i1, channel);

    const double w = 0.5 - 0.5 * std::cos(pi * frac);
    return n0 + (n1 - n0) * w; // [0, 1)
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RailProfile::generate(const std::string& traj_key,
                           double traj_length,
                           const std::vector<Irregularity>& explicit_irregularities,
                           const JointsParams& joints,
                           const ProceduralParams& noise,
                           Condition condition,
                           unsigned int seed)
{
    irregularities_.clear();
    noise_params_ = noise;
    condition_scale_ = conditionScale(condition);
    noise_seed_ = hashString(traj_key, seed);

    // Новый профиль - новая история тоннажа
    tonnage_bins_.clear();

    // Явные неровности, относящиеся к этому пути, - как есть
    irregularities_ = explicit_irregularities;

    // Автогенерация стыков: равномерный шаг, фаза и износ - детерминированы
    if (joints.joint_spacing > 1.0 && joints.joint_amplitude > 0.0)
    {
        const std::uint32_t phase_bits =
            mix32(noise_seed_ ^ 0x5bd1e995u);
        const double phase = static_cast<double>(phase_bits % 1000u) / 1000.0;

        const std::uint32_t wear_bits = mix32(noise_seed_ ^ 0x2545f491u);

        for (double coord = joints.joint_spacing * phase;
             coord < traj_length;
             coord += joints.joint_spacing)
        {
            Irregularity joint;
            joint.type = IrregularityType::RailJoint;
            joint.coord = coord;
            joint.amplitude = joints.joint_amplitude;
            joint.length = joints.joint_length;
            joint.side = RailSide::Both;

            // Износ стыка: детерминированный разброс 0.2..1.0
            const auto index = static_cast<std::uint32_t>(coord / std::max(joints.joint_spacing, 1.0));
            const double u = latticeNoise(index, wear_bits & 0xffffu);
            joint.wear = 0.2 + 0.8 * u;

            irregularities_.push_back(joint);
        }
    }

    // Автогенерация сварных швов между стыками
    if (joints.weld_spacing > 1.0 && joints.weld_amplitude > 0.0)
    {
        const std::uint32_t phase_bits = mix32(noise_seed_ ^ 0x68e31da4u);
        const double phase = static_cast<double>(phase_bits % 1000u) / 1000.0;

        const std::uint32_t quality_bits = mix32(noise_seed_ ^ 0x1b873593u);

        for (double coord = joints.weld_spacing * (0.5 + phase);
             coord < traj_length;
             coord += joints.weld_spacing)
        {
            Irregularity weld;
            weld.type = IrregularityType::Weld;
            weld.coord = coord;
            weld.amplitude = joints.weld_amplitude;
            weld.length = joints.weld_length;
            weld.side = RailSide::Both;

            const auto index = static_cast<std::uint32_t>(coord / std::max(joints.weld_spacing, 1.0));
            const double u = latticeNoise(index, quality_bits & 0xffffu);
            weld.quality = std::min(1.0, std::max(0.2, joints.weld_quality + 0.3 * (2.0 * u - 1.0)));

            irregularities_.push_back(weld);
        }
    }

    rebuildIndex();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RailProfile::addIrregularity(const Irregularity& irregularity)
{
    irregularities_.push_back(irregularity);
    rebuildIndex();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double RailProfile::railHeight(double path_coord, int side) const
{
    const double scale = condition_scale_ * track_damage_factor_ *
            (1.0 + tonnageWear(path_coord));

    return (explicitSum(path_coord, side) + noise(path_coord, side)) * scale;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double RailProfile::lateralOffset(double path_coord) const
{
    return lateralNoise(path_coord) * condition_scale_ *
            track_damage_factor_ * (1.0 + tonnageWear(path_coord));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RailProfile::isEmpty() const
{
    return irregularities_.empty() && (!noise_params_.enabled || condition_scale_ <= 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::size_t RailProfile::irregularityCount() const
{
    return irregularities_.size();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double RailProfile::noise(double path_coord, int side) const
{
    if (!noise_params_.enabled || noise_params_.wavelengths.empty())
        return 0.0;

    // Октавы чередуются: чётные - общие для обоих рельсов (просадка пути),
    // нечётные - зависят от стороны рельса (перекос, крутка пути)
    double sum = 0.0;

    for (std::size_t i = 0; i < noise_params_.wavelengths.size(); ++i)
    {
        const double gain = (i < noise_params_.octave_gains.size())
                ? noise_params_.octave_gains[i]
                : 1.0;

        const std::uint32_t channel = noise_seed_ ^
                ((i % 2 == 0)
                     ? 0x165667b1u
                     : static_cast<std::uint32_t>(side + 1) * 0x27d4eb2fu);

        const double octave = 2.0 * valueNoise(path_coord,
                                               noise_params_.wavelengths[i],
                                               channel) - 1.0; // [-1, 1)

        if (i % 2 == 0)
            sum += gain * noise_params_.amplitude * octave;
        else
            sum += gain * noise_params_.twist_amplitude * octave;
    }

    return sum;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double RailProfile::lateralNoise(double path_coord) const
{
    if (!noise_params_.enabled || noise_params_.lateral_wavelengths.empty())
        return 0.0;

    // Боковое смещение оси пути - свой канал детерминированного шума
    double sum = 0.0;

    for (std::size_t i = 0; i < noise_params_.lateral_wavelengths.size(); ++i)
    {
        const std::uint32_t channel = noise_seed_ ^
                (0x8da6b343u + 0x9e3779b9u * static_cast<std::uint32_t>(i));

        const double octave = 2.0 * valueNoise(path_coord,
                                               noise_params_.lateral_wavelengths[i],
                                               channel) - 1.0;

        sum += noise_params_.lateral_amplitude * octave;
    }

    return sum;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double RailProfile::explicitSum(double path_coord, int side) const
{
    if (sorted_by_begin_.empty())
        return 0.0;

    // Бинарный поиск: пропускаем неровности, начавшиеся правее точки
    const auto upper = std::upper_bound(sorted_by_begin_.begin(),
                                        sorted_by_begin_.end(),
                                        path_coord,
                                        [](double value, const Irregularity* item)
    {
        return value < item->beginCoord();
    });

    // Обратный проход по неровностям, начавшимся левее точки.
    // Вклад возможен, только пока начало не левее точки на max_length_
    const double search_floor = path_coord - max_length_;

    const auto side_enum = static_cast<RailSide>(side);

    double sum = 0.0;

    for (auto it = upper; it != sorted_by_begin_.begin(); --it)
    {
        const Irregularity* item = *(it - 1);

        if (item->beginCoord() < search_floor)
            break;

        if (item->side == RailSide::Both || item->side == side_enum)
            sum += item->offset(path_coord);
    }

    return sum;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RailProfile::degradeTrack(double factor)
{
    track_damage_factor_ = std::min(3.0, track_damage_factor_ + std::max(0.0, factor));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RailProfile::repairTrack()
{
    track_damage_factor_ = 1.0;

    // Капитальный ремонт: шлифовка/замена рельсов обнуляет тоннажный
    // износ участков (паттерн повреждений после схода)
    tonnage_bins_.clear();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RailProfile::addTonnage(double path_coord, double tonnes_per_axle_count)
{
    if (tonnes_per_axle_count <= 0.0 || tonnage_bin_size_ <= 0.0)
        return;

    // Центр участка сетки, в который попадает координата прохода
    const double bin_center = (std::floor(path_coord / tonnage_bin_size_) +
            0.5) * tonnage_bin_size_;

    // Сетка отсортирована по координате: ищем свой участок вставкой
    const auto it = std::lower_bound(tonnage_bins_.begin(),
                                     tonnage_bins_.end(),
                                     bin_center,
                                     [](const TonnageBin& bin, double value)
    {
        return bin.coord < value;
    });

    if (it != tonnage_bins_.end() && it->coord == bin_center)
    {
        it->tonnes += tonnes_per_axle_count;
    }
    else
    {
        TonnageBin bin;
        bin.coord = bin_center;
        bin.tonnes = tonnes_per_axle_count;
        tonnage_bins_.insert(it, bin);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double RailProfile::tonnageWear(double path_coord) const
{
    if (tonnage_bins_.empty())
        return 0.0;

    // Тоннаж берём из участка сетки под точкой
    const double tonnes = getTonnage(path_coord);

    // Линейный рост с насыщением: изношенный рельс не растёт вечно,
    // предел ограничивает рост амплитуд неровностей
    return std::min(tonnage_wear_rate_ * tonnes, tonnage_wear_limit_);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double RailProfile::getTonnage(double path_coord) const
{
    if (tonnage_bins_.empty() || tonnage_bin_size_ <= 0.0)
        return 0.0;

    const double bin_center = (std::floor(path_coord / tonnage_bin_size_) +
            0.5) * tonnage_bin_size_;

    const auto it = std::lower_bound(tonnage_bins_.begin(),
                                     tonnage_bins_.end(),
                                     bin_center,
                                     [](const TonnageBin& bin, double value)
    {
        return bin.coord < value;
    });

    if (it != tonnage_bins_.end() && it->coord == bin_center)
        return it->tonnes;

    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RailProfile::addPassage(double path_coord, double mass_tonnes,
                             double distance_m)
{
    if (mass_tonnes <= 0.0 || distance_m <= 0.0 ||
            tonnage_bin_size_ <= 0.0)
    {
        return;
    }

    // Доля участка, покрытая проходом: тоннаж распределяется на участок
    const double fraction = std::min(distance_m / tonnage_bin_size_, 1.0);
    addTonnage(path_coord, mass_tonnes * fraction);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double RailProfile::getTonnageWearLimit() const
{
    return tonnage_wear_limit_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double RailProfile::getTrackDamageFactor() const
{
    return track_damage_factor_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RailProfile::rebuildIndex()
{
    sorted_by_begin_.clear();
    sorted_by_begin_.reserve(irregularities_.size());

    max_length_ = 0.0;

    for (const Irregularity& item : irregularities_)
    {
        sorted_by_begin_.push_back(&item);
        max_length_ = std::max(max_length_, item.endCoord() - item.beginCoord());
    }

    std::sort(sorted_by_begin_.begin(), sorted_by_begin_.end(),
              [](const Irregularity* a, const Irregularity* b)
    {
        return a->beginCoord() < b->beginCoord();
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool loadTrackProfileConfig(const QString& route_dir,
                            TrackProfileConfig& out,
                            QString* error)
{
    out = TrackProfileConfig();

    const QString path = route_dir + "/track-profile.conf";

    CfgReader cfg;
    if (!cfg.load(path))
    {
        // Маршрут без конфига профилей - используем умолчания
        return true;
    }

    const QString sec = "TrackProfile";

    cfg.getBool(sec, "Enabled", out.enabled);

    int seed = static_cast<int>(out.seed);
    cfg.getInt(sec, "Seed", seed);
    out.seed = static_cast<unsigned int>(seed);

    QString condition_str = "normal";
    if (cfg.getString(sec, "Condition", condition_str))
    {
        bool ok = false;
        out.condition = conditionFromString(condition_str.toStdString().c_str(), &ok);
        if (!ok && error != nullptr)
            *error = QString("Unknown Condition level: %1").arg(condition_str);
    }

    cfg.getDouble(sec, "JointSpacing", out.joints.joint_spacing);
    cfg.getDouble(sec, "JointAmplitude", out.joints.joint_amplitude);
    cfg.getDouble(sec, "JointLength", out.joints.joint_length);
    cfg.getDouble(sec, "WeldSpacing", out.joints.weld_spacing);
    cfg.getDouble(sec, "WeldAmplitude", out.joints.weld_amplitude);
    cfg.getDouble(sec, "WeldLength", out.joints.weld_length);
    cfg.getDouble(sec, "WeldQuality", out.joints.weld_quality);

    cfg.getBool(sec, "NoiseEnabled", out.noise.enabled);
    cfg.getDouble(sec, "NoiseAmplitude", out.noise.amplitude);
    cfg.getDouble(sec, "TwistAmplitude", out.noise.twist_amplitude);
    cfg.getDouble(sec, "LateralNoiseAmplitude", out.noise.lateral_amplitude);

    // Неровности стрелочных переводов (крестовина) - автогенерация
    // на концах траекторий, подключённых к стрелкам
    cfg.getBool(sec, "SwitchIrregularity", out.switch_irregularity);
    cfg.getDouble(sec, "SwitchAmplitude", out.switch_amplitude);
    cfg.getDouble(sec, "SwitchLength", out.switch_length);

    // Зоны возвышения наружного рельса (Б16): секции [Cant] с полями
    // Trajectory / Begin / End / CantMm. Применяются топологией к трекам
    auto cant_node = cfg.getFirstSection("Cant");

    while (!cant_node.isNull())
    {
        CantZone zone;

        cfg.getString(cant_node, "Trajectory", zone.traj_name);
        cfg.getDouble(cant_node, "Begin", zone.begin);
        cfg.getDouble(cant_node, "End", zone.end);
        cfg.getDouble(cant_node, "CantMm", zone.cant_mm);

        if (zone.end >= zone.begin)
        {
            out.cants.push_back(zone);
        }

        cant_node = cfg.getNextSection();
    }

    // Явные неровности
    auto irregularity_node = cfg.getFirstSection("Irregularity");

    while (!irregularity_node.isNull())
    {
        QString traj_name = "";
        cfg.getString(irregularity_node, "Trajectory", traj_name);

        QString type_str = "dip";
        cfg.getString(irregularity_node, "Type", type_str);

        QString side_str = "both";
        cfg.getString(irregularity_node, "Side", side_str);

        Irregularity item;
        bool ok = false;
        item.type = Irregularity::typeFromString(type_str.toStdString().c_str(), &ok);
        if (!ok && error != nullptr)
        {
            *error = QString("Unknown irregularity type: %1").arg(type_str);
        }

        item.side = Irregularity::sideFromString(side_str.toStdString().c_str(), &ok);

        cfg.getDouble(irregularity_node, "Coord", item.coord);
        cfg.getDouble(irregularity_node, "Amplitude", item.amplitude);
        cfg.getDouble(irregularity_node, "Length", item.length);
        cfg.getDouble(irregularity_node, "EntryLength", item.entry_length);
        cfg.getDouble(irregularity_node, "ExitLength", item.exit_length);
        cfg.getDouble(irregularity_node, "Wavelength", item.wavelength);
        cfg.getDouble(irregularity_node, "Quality", item.quality);
        cfg.getDouble(irregularity_node, "Wear", item.wear);

        // Неровность без привязки к траектории применяется ко всем путям
        if (traj_name.isEmpty() || traj_name == "*")
        {
            out.explicit_irregularities.push_back(item);
        }
        else
        {
            named_irregularities.push_back({traj_name, item});
        }

        irregularity_node = cfg.getNextSection();
    }

    // Состояние отдельных траекторий
    auto condition_node = cfg.getFirstSection("TrajectoryCondition");

    while (!condition_node.isNull())
    {
        QString traj_name = "";
        cfg.getString(condition_node, "Trajectory", traj_name);

        QString level_str = "normal";
        cfg.getString(condition_node, "Level", level_str);

        bool ok = false;
        const Condition level =
                conditionFromString(level_str.toStdString().c_str(), &ok);
        if (ok && !traj_name.isEmpty())
        {
            out.traj_conditions.push_back({traj_name, level});
        }

        condition_node = cfg.getNextSection();
    }

    return true;
}

} // namespace track
