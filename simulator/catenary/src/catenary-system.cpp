//------------------------------------------------------------------------------
//
//      Catenary electrification system (КС, подстанции, секции)
//
//------------------------------------------------------------------------------

#include    "catenary-system.h"

#include    <CfgReader.h>
#include    <Journal.h>

#include    <algorithm>
#include    <cmath>

namespace catenary
{

namespace
{

SystemType systemFromString(const QString& text)
{
    if (text == "dc3")
        return SystemType::DC3kV;
    if (text == "ac25")
        return SystemType::AC25kV;
    if (text == "ac2x25")
        return SystemType::AC2x25kV;

    return SystemType::NotElectrified;
}

Phase phaseFromString(const QString& text)
{
    if (text == "A")
        return Phase::A;
    if (text == "B")
        return Phase::B;
    if (text == "C")
        return Phase::C;

    return Phase::None;
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CatenarySystem::load(const QString& route_dir)
{
    std::lock_guard<std::mutex> lock(mutex_);

    sections_.clear();
    substations_.clear();
    neutrals_.clear();

    CfgReader cfg;

    if (!cfg.load(route_dir + "/catenary.conf"))
    {
        // Неэлектрифицированный маршрут - допустимо
        return;
    }

    // Подстанции
    auto sub_node = cfg.getFirstSection("Substation");

    while (!sub_node.isNull())
    {
        Substation substation;

        cfg.getString(sub_node, "ID", substation.id);
        cfg.getDouble(sub_node, "RailwayCoord", substation.railway_coord);
        cfg.getDouble(sub_node, "Voltage", substation.nominal_voltage);
        cfg.getDouble(sub_node, "MaxPower", substation.max_power);
        cfg.getBool(sub_node, "SwitchedOn", substation.switched_on);
        cfg.getBool(sub_node, "RegenerativeReception", substation.regen_reception);
        cfg.getDouble(sub_node, "MaxRegenPower", substation.max_regen_power);

        substations_.push_back(substation);

        sub_node = cfg.getNextSection();
    }

    // Секции КС
    auto sec_node = cfg.getFirstSection("Section");

    while (!sec_node.isNull())
    {
        Section section;

        cfg.getDouble(sec_node, "Begin", section.begin);
        cfg.getDouble(sec_node, "End", section.end);

        QString type_str = "";
        if (cfg.getString(sec_node, "Type", type_str))
            section.type = systemFromString(type_str);

        QString phase_str = "";
        if (cfg.getString(sec_node, "Phase", phase_str))
            section.phase = phaseFromString(phase_str);

        cfg.getString(sec_node, "Substation", section.substation_id);
        cfg.getBool(sec_node, "SwitchedOn", section.switched_on);

        sections_.push_back(section);

        sec_node = cfg.getNextSection();
    }

    // Нейтральные вставки
    auto neutral_node = cfg.getFirstSection("NeutralInsert");

    while (!neutral_node.isNull())
    {
        NeutralInsert neutral;

        cfg.getDouble(neutral_node, "Begin", neutral.begin);
        cfg.getDouble(neutral_node, "End", neutral.end);

        neutrals_.push_back(neutral);

        neutral_node = cfg.getNextSection();
    }

    // Параметры сети
    double value = 0.0;
    if (cfg.getDouble("Catenary", "NetworkResistance", value))
        network_resistance = std::min(std::max(value, 0.01), 1.0);

    if (cfg.getDouble("Catenary", "BurnHeat", value))
        burn_heat_threshold = std::max(value, 1000.0);

    if (cfg.getDouble("Catenary", "CoolingRate", value))
        cooling_rate = std::max(value, 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const Section* CatenarySystem::findSection(double railway_coord) const
{
    for (const Section& section : sections_)
    {
        if (railway_coord >= section.begin && railway_coord <= section.end)
            return &section;
    }

    return nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const Substation* CatenarySystem::findSubstation(const QString& id) const
{
    for (const Substation& substation : substations_)
    {
        if (substation.id == id)
            return &substation;
    }

    return nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const NeutralInsert* CatenarySystem::findNeutral(double railway_coord) const
{
    for (const NeutralInsert& neutral : neutrals_)
    {
        if (railway_coord >= neutral.begin && railway_coord <= neutral.end)
            return &neutral;
    }

    return nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FeedState CatenarySystem::getFeedState(double railway_coord,
                                       double current_a) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    FeedState state;

    const NeutralInsert* neutral = findNeutral(railway_coord);

    if (neutral != nullptr)
    {
        // Нейтральная вставка: напряжения нет (ТЗ, п.10)
        state.in_neutral = true;
        state.powered = false;
        state.voltage = 0.0;
        return state;
    }

    const Section* section = findSection(railway_coord);

    if (section == nullptr || !section->switched_on || section->burned)
        return state;

    if (section->type == SystemType::NotElectrified)
        return state;

    state.type = section->type;
    state.phase = section->phase;
    state.substation_id = section->substation_id;

    const Substation* substation = findSubstation(section->substation_id);

    if (substation == nullptr || !substation->switched_on)
        return state;

    // Ток секции и подстанции учитывает все ПЕ, а не только эту:
    // тяжёлый состав просаживает напряжение сильнее
    const double effective_current = std::max(
        current_a,
        std::max(section->load_current, substation->load_current));

    // Провал напряжения: dU = I * r * d (п.6 - реальные расстояния)
    const double distance_km =
            std::abs(railway_coord - substation->railway_coord) / 1000.0;

    const double nominal = substation->nominal_voltage;

    double voltage = nominal;

    if (effective_current > 0.0)
    {
        const double drop = effective_current * network_resistance * distance_km;

        // Провал ограничен 30% номинала
        voltage -= std::min(drop, 0.3 * nominal);
    }

    // Ограничение мощности подстанции (п.5)
    const double power = voltage * effective_current;

    if (power > substation->max_power)
    {
        const double factor = substation->max_power / std::max(power, 1.0);
        voltage *= factor;
    }

    state.voltage = voltage;
    state.powered = true;

    // Приём рекуперации: подстанция с обратимым преобразователем
    // (ТЗ "Рекуперация", п.7)
    state.accepts_regen = substation->regen_reception;
    state.max_regen_power = substation->max_regen_power;

    return state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CatenarySystem::getCurrentLimit(double railway_coord) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    const Section* section = findSection(railway_coord);

    if (section == nullptr || section->burned)
        return 0.0;

    const Substation* substation = findSubstation(section->substation_id);

    if (substation == nullptr)
        return 0.0;

    return substation->max_power / std::max(substation->nominal_voltage, 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CatenarySystem::setSectionSwitched(double railway_coord, bool on)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (Section& section : sections_)
    {
        if (railway_coord >= section.begin && railway_coord <= section.end)
        {
            section.switched_on = on;

            // Восстановление провода после ремонта
            if (on)
            {
                section.burned = false;
                section.overload_heat = 0.0;
            }
        }
    }
}

//------------------------------------------------------------------------------
// Сбор нагрузок: вызывается из потока модели перед обходом ПЕ
//------------------------------------------------------------------------------
void CatenarySystem::beginLoadFrame()
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (Section& section : sections_)
    {
        section.load_current = 0.0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CatenarySystem::addSectionLoad(double railway_coord, double current_a)
{
    if (current_a <= 0.0)
        return;

    std::lock_guard<std::mutex> lock(mutex_);

    for (Section& section : sections_)
    {
        if (railway_coord >= section.begin && railway_coord <= section.end)
        {
            section.load_current += current_a;
            return;
        }
    }
}

//------------------------------------------------------------------------------
// Тепловая модель провода: перегруз копит А*с, при превышении
// порога провод пережигается и секция отключается
//------------------------------------------------------------------------------
void CatenarySystem::step(double dt)
{
    if (dt <= 0.0)
        return;

    std::lock_guard<std::mutex> lock(mutex_);

    for (Substation& substation : substations_)
    {
        substation.load_current = 0.0;
    }

    for (Section& section : sections_)
    {
        const double limit = [&]()
        {
            const Substation* substation = findSubstation(section.substation_id);
            if (substation == nullptr)
                return 0.0;

            return substation->max_power /
                   std::max(substation->nominal_voltage, 1.0);
        }();

        for (Substation& substation : substations_)
        {
            if (substation.id == section.substation_id)
            {
                substation.load_current += section.load_current;
            }
        }

        if (!section.switched_on || section.burned || limit <= 0.0)
            continue;

        const double overload = std::max(0.0, section.load_current - limit);

        if (overload > 0.0)
        {
            section.overload_heat += overload * dt;

            if (section.overload_heat >= burn_heat_threshold)
            {
                section.burned = true;
                section.switched_on = false;

                Journal::instance()->critical(
                    QString("[CATENARY] Contact wire burned at %1 m "
                            "(section %2..%3, load %4 A, limit %5 A)")
                    .arg(static_cast<int>(section.begin))
                    .arg(static_cast<int>(section.begin))
                    .arg(static_cast<int>(section.end))
                    .arg(static_cast<int>(section.load_current))
                    .arg(static_cast<int>(limit)));
            }
        }
        else
        {
            section.overload_heat = std::max(
                0.0, section.overload_heat - cooling_rate * dt);
        }
    }
}

} // namespace catenary
