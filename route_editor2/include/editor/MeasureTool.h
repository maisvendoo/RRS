#ifndef EDITOR_MEASURE_TOOL_H
#define EDITOR_MEASURE_TOOL_H

#include <vsg/maths/vec3.h>

#include <vector>

struct EditorContext;

namespace vsg
{

class ButtonPressEvent;

}

/**
 * @brief Инструмент измерения (клавиша M - вкл/выкл).
 *
 * Клики ЛКМ в активном режиме ставят точки по пересечению луча
 * камеры со сценой (существующий intersection_handler). Между
 * последовательными точками показываются длина и дельта Z в
 * ImGui-оверлее (правый нижний угол); сами точки и полилиния
 * рисуются по экранным проекциям через ImGui GetBackgroundDrawList.
 */
class MeasureTool
{
public:
    explicit MeasureTool(EditorContext& context);

    /// Вкл/выкл режим измерения (клавиша M)
    void set_active(bool active);

    bool is_active() const;

    /// Клик ЛКМ: поставить точку по пересечению со сценой
    void handle_press(const vsg::ButtonPressEvent& buttonPress);

    /// Очистить поставленные точки
    void clear();

    /// Отрисовка точек/линии и окна со длинами (каждый кадр)
    void draw_overlay();

private:
    EditorContext& context_;

    bool active_ = false;

    /// Поставленные точки (мировые координаты)
    std::vector<vsg::dvec3> points_;
};

#endif // EDITOR_MEASURE_TOOL_H
