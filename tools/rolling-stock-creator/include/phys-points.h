#ifndef PHYS_POINTS_H
#define PHYS_POINTS_H

#include <QList>
#include <QString>

//------------------------------------------------------------------------------
//
//  Справочник физических точек ПС (ТЗ п.14).
//  Типы соответствуют справочнику из ТЗ; координаты — в локальных
//  осях модели ПС (X — вправо, Y — вдоль пути, Z — вверх, метры).
//
//------------------------------------------------------------------------------
enum class PhysPointType
{
    Coupler,       ///< Точка автосцепки
    Wheel,         ///< Точка колесной пары
    Bogie,         ///< Точка тележки
    BrakeHose,     ///< Точка тормозного рукава
    EndValve,      ///< Точка концевого крана
    Pantograph,    ///< Точка токоприёмника
    Fuel,          ///< Точка заправки топливом
    Charging,      ///< Точка зарядки
    PassengerDoor, ///< Точка пассажирской двери
    DriverDoor,    ///< Точка двери машиниста
    Camera,        ///< Камера (позиция машиниста; пишется в [Cabine] XML)
    Sound,         ///< Источник звука
    Light,         ///< Источник света
    Spawn          ///< Точка появления
};

/// Одна физическая точка
struct PhysPoint
{
    QString name;             ///< Имя точки (уникальное в пределах проекта)
    PhysPointType type = PhysPointType::Coupler;
    double x = 0.0;           ///< Вправо, м
    double y = 0.0;           ///< Вдоль пути (вперёд), м
    double z = 0.0;           ///< Вверх, м
    double heading = 0.0;     ///< Направление (для Camera — DriverDir, град)

    bool operator==(const PhysPoint& other) const
    {
        return name == other.name && type == other.type &&
               x == other.x && y == other.y && z == other.z &&
               heading == other.heading;
    }
};

/// Строковый ключ типа (сохраняется в .trainproject)
inline QString pointTypeKey(PhysPointType type)
{
    switch (type)
    {
    case PhysPointType::Coupler:       return QStringLiteral("Coupler");
    case PhysPointType::Wheel:         return QStringLiteral("Wheel");
    case PhysPointType::Bogie:         return QStringLiteral("Bogie");
    case PhysPointType::BrakeHose:     return QStringLiteral("BrakeHose");
    case PhysPointType::EndValve:      return QStringLiteral("EndValve");
    case PhysPointType::Pantograph:    return QStringLiteral("Pantograph");
    case PhysPointType::Fuel:          return QStringLiteral("Fuel");
    case PhysPointType::Charging:      return QStringLiteral("Charging");
    case PhysPointType::PassengerDoor: return QStringLiteral("PassengerDoor");
    case PhysPointType::DriverDoor:    return QStringLiteral("DriverDoor");
    case PhysPointType::Camera:        return QStringLiteral("Camera");
    case PhysPointType::Sound:         return QStringLiteral("Sound");
    case PhysPointType::Light:         return QStringLiteral("Light");
    case PhysPointType::Spawn:         break;
    }

    return QStringLiteral("Spawn");
}

/// Русское отображаемое имя типа точки
inline QString pointTypeName(PhysPointType type)
{
    switch (type)
    {
    case PhysPointType::Coupler:       return QStringLiteral("Автосцепка");
    case PhysPointType::Wheel:         return QStringLiteral("Колесная пара");
    case PhysPointType::Bogie:         return QStringLiteral("Тележка");
    case PhysPointType::BrakeHose:     return QStringLiteral("Тормозной рукав");
    case PhysPointType::EndValve:      return QStringLiteral("Концевой кран");
    case PhysPointType::Pantograph:    return QStringLiteral("Токоприёмник");
    case PhysPointType::Fuel:          return QStringLiteral("Заправка топливом");
    case PhysPointType::Charging:      return QStringLiteral("Зарядка");
    case PhysPointType::PassengerDoor: return QStringLiteral("Пассажирская дверь");
    case PhysPointType::DriverDoor:    return QStringLiteral("Дверь машиниста");
    case PhysPointType::Camera:        return QStringLiteral("Камера (машинист)");
    case PhysPointType::Sound:         return QStringLiteral("Источник звука");
    case PhysPointType::Light:         return QStringLiteral("Источник света");
    case PhysPointType::Spawn:         break;
    }

    return QStringLiteral("Точка появления");
}

/// Все типы точек в порядке перечисления
inline QList<PhysPointType> allPointTypes()
{
    return
    {
        PhysPointType::Coupler,
        PhysPointType::Wheel,
        PhysPointType::Bogie,
        PhysPointType::BrakeHose,
        PhysPointType::EndValve,
        PhysPointType::Pantograph,
        PhysPointType::Fuel,
        PhysPointType::Charging,
        PhysPointType::PassengerDoor,
        PhysPointType::DriverDoor,
        PhysPointType::Camera,
        PhysPointType::Sound,
        PhysPointType::Light,
        PhysPointType::Spawn
    };
}

/// Тип по строковому ключу (неизвестный ключ -> Coupler)
inline PhysPointType pointTypeFromKey(const QString& key)
{
    const QList<PhysPointType> types = allPointTypes();

    for (const PhysPointType type : types)
    {
        if (pointTypeKey(type) == key)
        {
            return type;
        }
    }

    return PhysPointType::Coupler;
}

#endif // PHYS_POINTS_H
