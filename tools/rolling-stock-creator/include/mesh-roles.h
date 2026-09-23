#ifndef MESH_ROLES_H
#define MESH_ROLES_H

#include <QList>
#include <QString>

//------------------------------------------------------------------------------
//
//  Игровые роли объектов модели ПС (ТЗ п.3-4).
//  Роль определяет назначение меша для движка и используется
//  при генерации конфига коллизий и при экспорте.
//
//------------------------------------------------------------------------------
enum class MeshRole
{
    None = 0,   ///< Роль не назначена
    Body,       ///< Кузов
    Bogie1,     ///< Тележка 1
    Bogie2,     ///< Тележка 2
    Wheel,      ///< Колесная пара / колесо
    Trolley,    ///< Прочая тележка (без индивидуального номера)
    Door,       ///< Дверь
    Window,     ///< Окно
    Glass,      ///< Стекло
    Headlight,  ///< Фара / прожектор
    Taillight,  ///< Красный фонарь
    Pantograph, ///< Пантограф (токоприёмник)
    Coupler,    ///< Автосцепка
    BrakeHose,  ///< Тормозной рукав
    EndValve,   ///< Концевой кран
    Instrument, ///< Прибор
    Switch,     ///< Орган управления (кнопка/тумблер/рычаг)
    Cab,        ///< Кабина
    Interior,   ///< Интерьер / салон
    Seat,       ///< Пассажирское место
    Decor,      ///< Декоративный объект
    Other       ///< Другое
};

/// Строковый ключ роли (сохраняется в .trainproject)
inline QString meshRoleKey(MeshRole role)
{
    switch (role)
    {
    case MeshRole::Body:       return QStringLiteral("Body");
    case MeshRole::Bogie1:     return QStringLiteral("Bogie1");
    case MeshRole::Bogie2:     return QStringLiteral("Bogie2");
    case MeshRole::Wheel:      return QStringLiteral("Wheel");
    case MeshRole::Trolley:    return QStringLiteral("Trolley");
    case MeshRole::Door:       return QStringLiteral("Door");
    case MeshRole::Window:     return QStringLiteral("Window");
    case MeshRole::Glass:      return QStringLiteral("Glass");
    case MeshRole::Headlight:  return QStringLiteral("Headlight");
    case MeshRole::Taillight:  return QStringLiteral("Taillight");
    case MeshRole::Pantograph: return QStringLiteral("Pantograph");
    case MeshRole::Coupler:    return QStringLiteral("Coupler");
    case MeshRole::BrakeHose:  return QStringLiteral("BrakeHose");
    case MeshRole::EndValve:   return QStringLiteral("EndValve");
    case MeshRole::Instrument: return QStringLiteral("Instrument");
    case MeshRole::Switch:     return QStringLiteral("Switch");
    case MeshRole::Cab:        return QStringLiteral("Cab");
    case MeshRole::Interior:   return QStringLiteral("Interior");
    case MeshRole::Seat:       return QStringLiteral("Seat");
    case MeshRole::Decor:      return QStringLiteral("Decor");
    case MeshRole::Other:      return QStringLiteral("Other");
    case MeshRole::None:       break;
    }

    return QStringLiteral("None");
}

/// Русское отображаемое имя роли (для дерева объектов и комбобокса)
inline QString meshRoleName(MeshRole role)
{
    switch (role)
    {
    case MeshRole::Body:       return QStringLiteral("Кузов");
    case MeshRole::Bogie1:     return QStringLiteral("Тележка 1");
    case MeshRole::Bogie2:     return QStringLiteral("Тележка 2");
    case MeshRole::Wheel:      return QStringLiteral("Колесная пара");
    case MeshRole::Trolley:    return QStringLiteral("Тележка (прочая)");
    case MeshRole::Door:       return QStringLiteral("Дверь");
    case MeshRole::Window:     return QStringLiteral("Окно");
    case MeshRole::Glass:      return QStringLiteral("Стекло");
    case MeshRole::Headlight:  return QStringLiteral("Фара / прожектор");
    case MeshRole::Taillight:  return QStringLiteral("Красный фонарь");
    case MeshRole::Pantograph: return QStringLiteral("Пантограф");
    case MeshRole::Coupler:    return QStringLiteral("Автосцепка");
    case MeshRole::BrakeHose:  return QStringLiteral("Тормозной рукав");
    case MeshRole::EndValve:   return QStringLiteral("Концевой кран");
    case MeshRole::Instrument: return QStringLiteral("Прибор");
    case MeshRole::Switch:     return QStringLiteral("Орган управления");
    case MeshRole::Cab:        return QStringLiteral("Кабина");
    case MeshRole::Interior:   return QStringLiteral("Интерьер / салон");
    case MeshRole::Seat:       return QStringLiteral("Пассажирское место");
    case MeshRole::Decor:      return QStringLiteral("Декоративный объект");
    case MeshRole::Other:      return QStringLiteral("Другое");
    case MeshRole::None:       break;
    }

    return QStringLiteral("Без роли");
}

/// Все роли в порядке перечисления (первый элемент — None)
inline QList<MeshRole> allMeshRoles()
{
    return
    {
        MeshRole::None,
        MeshRole::Body,
        MeshRole::Bogie1,
        MeshRole::Bogie2,
        MeshRole::Wheel,
        MeshRole::Trolley,
        MeshRole::Door,
        MeshRole::Window,
        MeshRole::Glass,
        MeshRole::Headlight,
        MeshRole::Taillight,
        MeshRole::Pantograph,
        MeshRole::Coupler,
        MeshRole::BrakeHose,
        MeshRole::EndValve,
        MeshRole::Instrument,
        MeshRole::Switch,
        MeshRole::Cab,
        MeshRole::Interior,
        MeshRole::Seat,
        MeshRole::Decor,
        MeshRole::Other
    };
}

/// Роль по строковому ключу (nullptr-безопасно: неизвестный ключ -> None)
inline MeshRole meshRoleFromKey(const QString& key)
{
    const QList<MeshRole> roles = allMeshRoles();

    for (const MeshRole role : roles)
    {
        if (meshRoleKey(role) == key)
        {
            return role;
        }
    }

    return MeshRole::None;
}

//------------------------------------------------------------------------------
/// Эвристика автоопределения роли по имени объекта/коллекции (ТЗ п.3).
/// Сравнение регистронезависимое, ищется вхождение ключевого слова.
//------------------------------------------------------------------------------
inline MeshRole guessMeshRole(const QString& object_name)
{
    const QString name = object_name.toLower();

    struct KeywordRole
    {
        const char* keyword;
        MeshRole role;
    };

    // Порядок важен: более специфичные ключевые слова идут раньше
    static const KeywordRole table[] =
    {
        {"пантограф",     MeshRole::Pantograph},
        {"токоприемник",  MeshRole::Pantograph},
        {"токоприёмник",  MeshRole::Pantograph},
        {"pantograph",    MeshRole::Pantograph},
        {"current",       MeshRole::Pantograph},
        {"тележка_1",     MeshRole::Bogie1},
        {"тележка-1",     MeshRole::Bogie1},
        {"тележка 1",     MeshRole::Bogie1},
        {"тележка1",      MeshRole::Bogie1},
        {"bogie1",        MeshRole::Bogie1},
        {"bogie_1",       MeshRole::Bogie1},
        {"тележка_2",     MeshRole::Bogie2},
        {"тележка-2",     MeshRole::Bogie2},
        {"тележка 2",     MeshRole::Bogie2},
        {"тележка2",      MeshRole::Bogie2},
        {"bogie2",        MeshRole::Bogie2},
        {"bogie_2",       MeshRole::Bogie2},
        {"тележка",       MeshRole::Trolley},
        {"тележек",       MeshRole::Trolley},
        {"bogie",         MeshRole::Trolley},
        {"truck",         MeshRole::Trolley},
        {"wheelset",      MeshRole::Wheel},
        {"колесная",      MeshRole::Wheel},
        {"колесо",        MeshRole::Wheel},
        {"колёсн",        MeshRole::Wheel},
        {"ось",           MeshRole::Wheel},
        {"wheel",         MeshRole::Wheel},
        {"axle",          MeshRole::Wheel},
        {"brakehose",     MeshRole::BrakeHose},
        {"рукав",         MeshRole::BrakeHose},
        {"hose",          MeshRole::BrakeHose},
        {"концевой",      MeshRole::EndValve},
        {"endvalve",      MeshRole::EndValve},
        {"anglecock",     MeshRole::EndValve},
        {"кран",          MeshRole::EndValve},
        {"valve",         MeshRole::EndValve},
        {"автосцепк",     MeshRole::Coupler},
        {"сцепк",         MeshRole::Coupler},
        {"coupler",       MeshRole::Coupler},
        {"sa3",           MeshRole::Coupler},
        {"кузов",         MeshRole::Body},
        {"корпус",        MeshRole::Body},
        {"body",          MeshRole::Body},
        {"carriage",      MeshRole::Body},
        {"рама",          MeshRole::Body},
        {"двер",          MeshRole::Door},
        {"door",          MeshRole::Door},
        {"стекл",         MeshRole::Glass},
        {"glass",         MeshRole::Glass},
        {"окн",           MeshRole::Window},
        {"window",        MeshRole::Window},
        {"фар",           MeshRole::Headlight},
        {"прожектор",     MeshRole::Headlight},
        {"headlight",     MeshRole::Headlight},
        {"фонар",         MeshRole::Taillight},
        {"taillight",     MeshRole::Taillight},
        {"redlight",      MeshRole::Taillight},
        {"кабин",         MeshRole::Cab},
        {"cab",           MeshRole::Cab},
        {"прибор",        MeshRole::Instrument},
        {"gauge",         MeshRole::Instrument},
        {"манометр",      MeshRole::Instrument},
        {"скоростемер",   MeshRole::Instrument},
        {"кнопк",         MeshRole::Switch},
        {"тумблер",       MeshRole::Switch},
        {"переключател",  MeshRole::Switch},
        {"рычаг",         MeshRole::Switch},
        {"button",        MeshRole::Switch},
        {"switch",        MeshRole::Switch},
        {"lever",         MeshRole::Switch},
        {"салон",         MeshRole::Interior},
        {"интерьер",      MeshRole::Interior},
        {"внутр",         MeshRole::Interior},
        {"interior",      MeshRole::Interior},
        {"место",         MeshRole::Seat},
        {"сиден",         MeshRole::Seat},
        {"seat",          MeshRole::Seat},
        {"декор",         MeshRole::Decor},
        {"decor",         MeshRole::Decor}
    };

    for (const KeywordRole& entry : table)
    {
        if (name.contains(QLatin1String(entry.keyword)))
        {
            return entry.role;
        }
    }

    return MeshRole::None;
}

#endif // MESH_ROLES_H
