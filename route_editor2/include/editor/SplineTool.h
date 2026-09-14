#ifndef EDITOR_SPLINE_TOOL_H
#define EDITOR_SPLINE_TOOL_H

#include <vsg/maths/vec3.h>

#include <vsg/ui/PointerEvent.h>

#include <cstddef>
#include <vector>

struct EditorContext;

namespace vsg
{

class ButtonPressEvent;

}

/**
 * @brief Инструмент построения новых путей (клавиша N - вкл/выкл).
 *
 * Клики ЛКМ ставят опорные точки по пересечению луча камеры со сценой
 * (существующий intersection_handler, как в MeasureTool). По опорным
 * точкам считается кривая Catmull-Rom (20 сегментов на интервал) и
 * рисуется по экранным проекциям через ImGui GetBackgroundDrawList.
 * Окно «Новый путь» показывает список точек с длинами сегментов,
 * поле имени пути и кнопку «Создать путь»: запись ProposedTrack
 * уходит в track-edit.conf, а вдоль сплайна строится геометрия пути
 * (TrackFurniture::build_track_geometry, сэмплы с шагом 5 м).
 */
class SplineTool
{
public:
    explicit SplineTool(EditorContext& context);

    /// Вкл/выкл режим построения пути (клавиша N)
    void set_active(bool active);

    bool is_active() const;

    /// Клик ЛКМ: поставить опорную точку (снап к концу пути, иначе -
    /// пересечение луча с плоскостью земли z=0)
    void handle_press(const vsg::ButtonPressEvent& buttonPress);

    /// Движение мыши: позиция курсора для живого превью сегмента
    void handle_move(const vsg::MoveEvent& moveEvent);

    /// Удалить последнюю опорную точку (Backspace);
    /// false - точек уже нет
    bool remove_last();

    /// Очистить поставленные точки
    void clear();

    /// Превью-кривая + окно «Новый путь» (каждый кадр)
    void draw_overlay();

    /// Точки кривой Catmull-Rom по опорным точкам: равномерный
    /// сплайн, крайние опорные точки дублируются, по
    /// segments_per_interval сегментов на каждый интервал
    static std::vector<vsg::dvec3> catmull_rom_points(
        const std::vector<vsg::dvec3>& control_points,
        std::size_t segments_per_interval);

    /// Сэмплы кривой Catmull-Rom с равномерным шагом step_m по длине
    /// кривой (конечная точка включается всегда)
    static std::vector<vsg::dvec3> sample_spline_by_step(
        const std::vector<vsg::dvec3>& control_points, double step_m);

private:
    /// «Создать путь»: ProposedTrack в track-edit.conf + TrackMesh
    /// вдоль сплайна
    bool create_track();

    EditorContext& context_;

    bool active_ = false;

    /// Поставленные опорные точки (мировые координаты)
    std::vector<vsg::dvec3> points_;

    /// Имя нового пути (поле окна «Новый путь»)
    char track_name_[128] = {};

    /// Мировая точка под курсором (для превью сегмента, z=0)
    vsg::dvec3 cursor_world_ = {0.0, 0.0, 0.0};
    bool has_cursor_ = false;

    /// Прокладка секциями (TSRE-style): 0 - свободный сплайн,
    /// далее шаблоны прямых/кривых из таблицы SplineTool.cpp
    int section_index_ = 0;

    /// Текущее направление прокладки, град (0 = север/+Y)
    double heading_deg_ = 0.0;
};

#endif // EDITOR_SPLINE_TOOL_H
