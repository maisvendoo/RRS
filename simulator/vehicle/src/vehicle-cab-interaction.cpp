//------------------------------------------------------------------------------
//
//      Cab interaction (универсальная система интерактивной кабины)
//
//------------------------------------------------------------------------------

#include    "vehicle-cab-interaction.h"

#include    <CfgReader.h>

#include    <algorithm>
#include    <utility>

namespace
{

/// Имя типа элемента -> тип (Toggle/Button/Lever/Valve/Gauge)
bool elementTypeFromName(const QString& text, CabElementType& type)
{
    QString name = text.toLower();
    name.remove('_');
    name.remove(' ');

    if (name == "toggle")   { type = CabElementType::Toggle; return true; }
    if (name == "button")   { type = CabElementType::Button; return true; }
    if (name == "lever")    { type = CabElementType::Lever; return true; }
    if (name == "valve")    { type = CabElementType::Valve; return true; }
    if (name == "gauge")    { type = CabElementType::Gauge; return true; }

    return false;
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CabInteractionRegistry::loadConfig(QString cfg_path)
{
    CfgReader cfg;

    if (!cfg.load(cfg_path))
        return;

    auto node = cfg.getFirstSection("CabElement");

    while (!node.isNull())
    {
        Item item;

        cfg.getInt(node, "ControlID", item.id);
        cfg.getString(node, "Name", item.name);
        cfg.getString(node, "Hint", item.hint);
        cfg.getBool(node, "IsToggle", item.is_toggle);

        //--- Расширенные ключи (ТЗ "Взаимодействие с кабиной") ---

        // Имя меша для сопоставления с элементом (пикинг вьювера)
        cfg.getString(node, "ModelName", item.model_name);

        // Тип элемента; по умолчанию - Toggle (совместимость)
        QString type_str = "";
        if (cfg.getString(node, "Type", type_str) &&
                elementTypeFromName(type_str, item.type))
        {
            // Дискретные типы - тумблер/кнопка, остальные непрерывные
            item.is_toggle = (item.type == CabElementType::Toggle) ||
                             (item.type == CabElementType::Button);
        }

        // Непрерывное состояние и границы (для Lever/Valve/Gauge)
        cfg.getDouble(node, "MinState", item.min_state);
        cfg.getDouble(node, "MaxState", item.max_state);
        cfg.getBool(node, "Continuous", item.continuous);

        // Прибор по умолчанию не интерактивен
        item.interactable = (item.type != CabElementType::Gauge);
        cfg.getBool(node, "Interactable", item.interactable);

        // Звук и анимация
        cfg.getInt(node, "SoundID", item.sound_id);
        cfg.getString(node, "AnimName", item.anim_name);

        // Границы могли прийти в обратном порядке
        if (item.max_state < item.min_state)
            std::swap(item.min_state, item.max_state);

        // Стартовое непрерывное состояние - нижняя граница
        item.value = item.min_state;

        if (item.id != 0)
            items_.push_back(item);

        node = cfg.getNextSection();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::size_t CabInteractionRegistry::count() const
{
    return items_.size();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const CabInteractionRegistry::Item& CabInteractionRegistry::getItem(std::size_t index) const
{
    return items_[index];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int CabInteractionRegistry::findByControlId(int control_id) const
{
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        if (items_[i].id == control_id)
            return static_cast<int>(i);
    }

    return -1;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int CabInteractionRegistry::findByModelName(const QString& model_name) const
{
    // Сопоставление меш <-> элемент (raycast-пикинг вьювера):
    // сначала точное совпадение с явным ключом ModelName,
    // затем - с именем элемента Name (без ModelName)
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        if (!items_[i].model_name.isEmpty() &&
                items_[i].model_name.compare(model_name,
                                             Qt::CaseInsensitive) == 0)
            return static_cast<int>(i);
    }

    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        if (items_[i].model_name.isEmpty() &&
                items_[i].name.compare(model_name,
                                       Qt::CaseInsensitive) == 0)
            return static_cast<int>(i);
    }

    return -1;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CabInteractionRegistry::setState(std::size_t index, bool state)
{
    if (index >= items_.size())
        return;

    Item& item = items_[index];

    item.state = state;

    // Синхронизация непрерывного состояния: концы диапазона
    item.value = state ? item.max_state : item.min_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CabInteractionRegistry::setValue(std::size_t index, double value)
{
    if (index >= items_.size())
        return;

    Item& item = items_[index];

    // Зажим в границы диапазона; дискретное состояние - по середине
    item.value = std::min(std::max(value, item.min_state), item.max_state);
    item.state = item.value > (item.min_state + item.max_state) / 2.0;
}
