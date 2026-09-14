#ifndef EDITOR_VALIDATOR_H
#define EDITOR_VALIDATOR_H

#include <vsg/maths/vec3.h>

#include <string>
#include <vector>

struct EditorContext;

/// Тип находки валидации
enum class ValidationType
{
    /// Ошибка: маршрут работает неправильно
    Error,
    /// Предупреждение: возможная проблема
    Warning
};

/// Одна находка валидации маршрута
struct ValidationIssue
{
    ValidationType type = ValidationType::Warning;
    std::string text;

    /// Точка для телепорта камеры (кнопка Go)
    vsg::dvec3 position = {0.0, 0.0, 0.0};
    bool has_position = false;
};

/**
 * @brief Валидатор маршрута (окно «Валидация», промт п.33).
 *
 * Проверки по данным EditorContext:
 *  - объекты route1.map с label, отсутствующим в objects.ref;
 *  - NaN/Inf в translation/rotation/scale объектов;
 *  - пары объектов ближе 0.5 м (вероятные дубликаты);
 *  - станции без позиции;
 *  - топология не загружена.
 */
class Validator
{
public:
    /// Выполнить все проверки и вернуть список находок
    static std::vector<ValidationIssue> validate(EditorContext& context);

    /**
     * @brief Проверка габаритов (промт п.25, упрощённо): объекты,
     *        чей план-габарит пересекает цилиндр радиуса 2.5 м
     *        вокруг оси каждой траектории (точки оси сэмплируются
     *        каждые 10 м). Возвращает предупреждения вида
     *        «Объект %1 близко к пути %2» с точкой для телепорта.
     */
    static std::vector<ValidationIssue> check_clearance(
        EditorContext& context);
};

#endif // EDITOR_VALIDATOR_H
