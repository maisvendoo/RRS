//------------------------------------------------------------------------------
//
//      Character appearance (внешний вид персонажа)
//
//------------------------------------------------------------------------------

#include    "character-appearance.h"

#include    <CfgReader.h>

#include    <algorithm>
#include    <chrono>
#include    <cstdint>
#include <random>

namespace
{

const QStringList kSlots = {"head", "hair", "torso", "legs", "feet",
                            "hat", "glasses", "gloves", "badge"};

std::uint32_t mix32(std::uint32_t value)
{
    value ^= value >> 16;
    value *= 0x85ebca6bu;
    value ^= value >> 13;
    value *= 0xc2b2ae35u;
    value ^= value >> 16;
    return value;
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CharacterAppearance::loadConfig(QString cfg_path)
{
    CfgReader cfg;

    if (!cfg.load(cfg_path))
        return;

    QString gender_str = "";
    if (cfg.getString("Character", "Gender", gender_str))
    {
        gender_ = (gender_str == "female") ? Gender::Female : Gender::Male;
    }

    QString uniform = "";
    if (cfg.getString("Character", "Uniform", uniform) && !uniform.isEmpty())
    {
        Item item;
        // Префикс "uniform/" в id - признак железнодорожной формы (п.7),
        // по нему работает isInRailwayUniform()
        item.id = "uniform/" + uniform;
        item.model = item.id;
        equip("torso", item);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CharacterAppearance::setGender(Gender gender)
{
    if (gender_ == gender)
        return;

    gender_ = gender;

    // Предметы, зависящие от пола, снимаются (упрощённо: волосы/торс)
    unequip("hair");
    unequip("torso");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CharacterAppearance::equip(const QString& slot, const Item& item)
{
    // Слота нет - нельзя
    if (!slotList().contains(slot))
        return false;

    // Предмет скрывает слоты (шапка прячет волосы, п.10)
    for (const QString& hidden : item.hides)
    {
        unequip(hidden);
    }

    // Существующий предмет может прятать новый
    for (auto& pair : equipment_)
    {
        if (pair.second.hides.contains(slot))
            return false;
    }

    // Замена
    for (auto& pair : equipment_)
    {
        if (pair.first == slot)
        {
            pair.second = item;
            return true;
        }
    }

    equipment_.emplace_back(slot, item);
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CharacterAppearance::unequip(const QString& slot)
{
    equipment_.erase(
                std::remove_if(equipment_.begin(), equipment_.end(),
                               [&slot](const std::pair<QString, Item>& pair)
    {
        return pair.first == slot;
    }),
                equipment_.end());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const CharacterAppearance::Item& CharacterAppearance::getEquipped(
        const QString& slot) const
{
    for (const auto& pair : equipment_)
    {
        if (pair.first == slot)
            return pair.second;
    }

    return empty_item_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CharacterAppearance::isInRailwayUniform() const
{
    const Item& torso = getEquipped("torso");

    return torso.id.startsWith("uniform/");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CharacterAppearance::randomize()
{
    const std::uint32_t seed = mix32(
                static_cast<std::uint32_t>(
                    std::chrono::system_clock::now().time_since_epoch().count()));

    std::mt19937 gen(seed);

    gender_ = (gen() % 2 == 0) ? Gender::Male : Gender::Female;

    equipment_.clear();

    // Базовый случайный набор: волосы + торс + ноги + обувь
    Item hair;
    hair.id = QString("hair_%1").arg(gen() % 6);
    hair.model = "hair/" + hair.id;
    equip("hair", hair);

    Item torso;
    const size_t color_idx = gen() % static_cast<size_t>(
                std::max<size_t>(uniform_colors_.size(), 1));
    const QString& color = uniform_colors_[static_cast<int>(color_idx)];
    torso.id = "uniform/worker_" + color;
    torso.model = torso.id;
    torso.material = color;    // Цветовой вариант формы (п.8)
    equip("torso", torso);

    Item legs;
    legs.id = "pants_work";
    legs.model = "legs/pants_work";
    equip("legs", legs);

    Item feet;
    feet.id = "boots";
    feet.model = "feet/boots";
    equip("feet", feet);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString CharacterAppearance::serialize() const
{
    QString data;

    data += QString("gender=%1;").arg(
                gender_ == Gender::Female ? "female" : "male");

    for (const auto& pair : equipment_)
    {
        data += QString("%1=%2:%3;")
                .arg(pair.first)
                .arg(pair.second.id)
                .arg(pair.second.material);
    }

    return data;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CharacterAppearance::deserialize(const QString& data)
{
    const QStringList parts = data.split(';', Qt::SkipEmptyParts);

    for (const QString& part : parts)
    {
        const QStringList key_value = part.split('=');

        if (key_value.size() != 2)
            continue;

        const QString& key = key_value[0];
        const QString& value = key_value[1];

        if (key == "gender")
        {
            gender_ = (value == "female") ? Gender::Female : Gender::Male;
            continue;
        }

        const QStringList id_mat = value.split(':');

        Item item;
        item.id = id_mat[0];
        item.model = id_mat[0];

        if (id_mat.size() > 1)
            item.material = id_mat[1];

        equip(key, item);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CharacterAppearance::Gender CharacterAppearance::getGender() const
{
    return gender_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QStringList& CharacterAppearance::slotList()
{
    return kSlots;
}
