#ifndef VEHICLE_SCHEMA_H
#define VEHICLE_SCHEMA_H

#include <QString>
#include <QList>

#include <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
/// Тип поля конфигурации ПС
enum class FieldType
{
    Double,
    Int,
    String,
    Bool
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
/// Описание одного поля секции конфигурации ПС
struct FieldSpec
{
    /// Имя ключа (XML-тег)
    QString key;

    /// Тип значения
    FieldType type = FieldType::Double;

    /// Минимально допустимое значение (для Double/Int)
    double min_value = 0.0;

    /// Максимально допустимое значение (для Double/Int)
    double max_value = 0.0;

    /// Значение по умолчанию (строковое представление)
    QString default_value;

    /// Человеческое описание поля
    QString description;

    /// Обязательный ключ (проверяется при сохранении)
    bool required = false;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
/// Описание секции конфигурации ПС
struct SectionSpec
{
    /// Имя секции (XML-тег верхнего уровня)
    QString name;

    /// Секция может повторяться в файле несколько раз (например CabElement)
    bool multiple = false;

    /// Поля секции
    QList<FieldSpec> fields;
};

/// Таблица схемы конфигурации ПС: секции и их поля
const std::vector<SectionSpec>& vehicleSchema();

/// Найти описание секции по имени (nullptr, если не найдена)
const SectionSpec* findSectionSpec(const QString& name);

#endif // VEHICLE_SCHEMA_H
