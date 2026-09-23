//------------------------------------------------------------------------------
//
//      Catenary electrification system (КС, подстанции, секции)
//
//------------------------------------------------------------------------------

#include    "catenary-system.h"

#include    <CfgReader.h>

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

    if (section == nullptr || !section->switched_on)
        return state;

    if (section->type == SystemType::NotElectrified)
        return state;

    state.type = section->type;
    state.phase = section->phase;
    state.substation_id = section->substation_id;

    const Substation* substation = findSubstation(section->substation_id);

    if (substation == nullptr || !substation->switched_on)
        return state;

    // Провал напряжения: dU = I * r * d (п.6 - реальные расстояния)
    const double distance_km =
            std::abs(railway_coord - substation->railway_coord) / 1000.0;

    const double nominal = substation->nominal_voltage;

    double voltage = nominal;

    if (current_a > 0.0)
    {
        const double drop = current_a * network_resistance * distance_km;

        // Провал ограничен 30% номинала
        voltage -= std::min(drop, 0.3 * nominal);
    }

    // Ограничение мощности подстанции (п.5)
    const double power = voltage * current_a;

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
    const Section* section = findSection(railway_coord);

    if (section == nullptr)
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
    for (Section& section : sections_)
    {
        if (railway_coord >= section.begin && railway_coord <= section.end)
            section.switched_on = on;
    }
}

} // namespace catenary
