#ifndef EDITOR_GIZMO_H
#define EDITOR_GIZMO_H

#include "editor/SingleSwitch.h"
#include "editor/settings/GizmoSettings.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsg/utils/Builder.h>

struct EditorContext;

namespace vsg
{

class ButtonPressEvent;
class ButtonReleaseEvent;
class MatrixTransform;
class MoveEvent;
class Node;

}

/// Режимы гизмо (клавиша G циклически их переключает)
enum class GizmoMode
{
    /// Гизмо выключено
    OFF,
    /// Перемещение вдоль осей (стрелки)
    TRANSLATE,
    /// Поворот вокруг осей (кольца)
    ROTATE,
    /// Масштабирование вдоль осей (оси с кубиками)
    SCALE
};

/**
 * @brief Гизмо трансформации выделенных объектов (порт из старого
 *        редактора, расширенный режимами поворота и масштаба).
 *
 * Включается клавишей G (цикл translate -> rotate -> scale -> off).
 * ЛКМ по оси/кольцу захватывает гизмо, перетаскивание применяется
 * к выделенным объектам через те же вызовы, что использует инспектор
 * (RouteObject::move / rotate_around_pivot / scale_relative_to_pivot);
 * при отпускании записывается команда undo (TranslateObjects /
 * RotateObjects / ScaleObjects).
 *
 * Пикинг выполняется своими лучами через IntersectionHandler (маска
 * MASK_CLICKABLE), плоскости перетаскивания - огромные квадры,
 * включаемые только на время перетаскивания (как в старом редакторе).
 */
class Gizmo : public vsg::Inherit<SingleSwitch, Gizmo>
{
public:
    Gizmo(EditorContext& context, const gizmo_settings_t& gizmo_settings);

    /// Попытка захвата гизмо по нажатию ЛКМ (true - захватили)
    bool handle_press(const vsg::ButtonPressEvent& buttonPress);

    /// Завершение перетаскивания: запись команды undo
    void handle_release(const vsg::ButtonReleaseEvent& buttonRelease);

    /// Перетаскивание (вызывается на каждое движение мыши)
    void handle_move(const vsg::MoveEvent& moveEvent);

    /// Обновление позиции/масштаба/видимости (каждый кадр)
    void update();

    void set_mode(GizmoMode mode);

    GizmoMode get_mode() const;

    /// Следующий режим цикла translate -> rotate -> scale -> off
    static GizmoMode next_mode(GizmoMode mode);

    const vsg::dvec3& get_curr_pos() const;

private:
    /// Ось с индексом 0/1/2 как вектор
    static vsg::dvec3 axis_vector(int index);

private:
    EditorContext& context_;
    const gizmo_settings_t& gizmo_settings;

    vsg::Builder builder_;
    vsg::ref_ptr<vsg::MatrixTransform> matrix_transform_;

    /// Ветви трёх режимов (включается только активная)
    vsg::ref_ptr<SingleSwitch> translate_switch_;
    vsg::ref_ptr<SingleSwitch> rotate_switch_;
    vsg::ref_ptr<SingleSwitch> scale_switch_;

    /// Стрелки перемещения (кликабельны в режиме translate)
    vsg::ref_ptr<vsg::Node> arrow_x_;
    vsg::ref_ptr<vsg::Node> arrow_y_;
    vsg::ref_ptr<vsg::Node> arrow_z_;

    /// Кольца поворота (кликабельны в режиме rotate)
    vsg::ref_ptr<vsg::Node> ring_x_;
    vsg::ref_ptr<vsg::Node> ring_y_;
    vsg::ref_ptr<vsg::Node> ring_z_;

    /// Оси масштабирования (кликабельны в режиме scale)
    vsg::ref_ptr<vsg::Node> scale_x_;
    vsg::ref_ptr<vsg::Node> scale_y_;
    vsg::ref_ptr<vsg::Node> scale_z_;

    /// Огромные плоскости-приёмники перетаскивания (выключены)
    vsg::ref_ptr<SingleSwitch> plane_yz_switch_;
    vsg::ref_ptr<SingleSwitch> plane_xz_switch_;
    vsg::ref_ptr<SingleSwitch> plane_xy_switch_;

    /// Линии-подсветки активной оси (выключены)
    vsg::ref_ptr<SingleSwitch> line_x_switch_;
    vsg::ref_ptr<SingleSwitch> line_y_switch_;
    vsg::ref_ptr<SingleSwitch> line_z_switch_;

    GizmoMode mode_ = GizmoMode::OFF;
    bool dragging_ = false;

    vsg::dvec3 curr_pos_ = {0.0, 0.0, 0.0};
    double scale_ = 1.0;

    /// Состояние перетаскивания
    int drag_axis_index_ = -1;
    vsg::dvec3 drag_axis_ = {0.0, 0.0, 1.0};
    vsg::dvec3 drag_pivot_ = {0.0, 0.0, 0.0};
    double drag_length_unit_ = 1.0;
    vsg::dvec3 prev_intersect_pos_ = {0.0, 0.0, 0.0};

    /// Накопленные трансформации за время перетаскивания
    vsg::dvec3 raw_total_translation_ = {0.0, 0.0, 0.0};
    vsg::dvec3 applied_translation_ = {0.0, 0.0, 0.0};
    double total_radians_ = 0.0;
    double total_scale_ = 1.0;

    vsg::ref_ptr<SingleSwitch> active_plane_switch_;
    vsg::ref_ptr<SingleSwitch> active_line_switch_;
};

#endif // EDITOR_GIZMO_H
