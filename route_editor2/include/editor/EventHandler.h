#ifndef EDITOR_EVENT_HANDLER_H
#define EDITOR_EVENT_HANDLER_H

#include "editor/Gizmo.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>

struct EditorContext;
class Keyboard;

namespace vsg
{

class ButtonPressEvent;
class ButtonReleaseEvent;
class FocusInEvent;
class FocusOutEvent;
class FrameEvent;
class KeyPressEvent;
class KeyReleaseEvent;
class MoveEvent;
class ScrollWheelEvent;

}

/**
 * @brief Центральный обработчик событий редактора.
 *
 * Камера-полёт по образцу CameraHandler старого редактора:
 * WASD — движение вперёд/назад/вбок при зажатой ПКМ,
 * Q/E — вниз/вверх, колесо — зум (Ctrl+колесо — скорость),
 * ЛКМ — выбор объектов, Delete — удаление,
 * Ctrl+Z/Ctrl+Y — undo/redo, Ctrl+S — сохранение route1.map.
 *
 * Команды берутся из таблицы переназначаемых клавиш
 * (context.key_bindings, окно «Клавиши»); P/Esc режима «Пути»
 * и клавиши инструментов G/R/T (гизмо), M (измерение),
 * N (новый путь), Backspace (удалить точку пути),
 * X/[/] (привязка к сетке) остались прямыми.
 */
class EventHandler : public vsg::Inherit<vsg::Visitor, EventHandler>
{
public:
    explicit EventHandler(EditorContext& context);
    ~EventHandler();

    virtual void apply(vsg::KeyPressEvent& keyPress) override;
    virtual void apply(vsg::KeyReleaseEvent& keyRelease) override;
    virtual void apply(vsg::FocusInEvent& focusIn) override;
    virtual void apply(vsg::FocusOutEvent& focusOut) override;

    virtual void apply(vsg::ButtonPressEvent& buttonPress) override;
    virtual void apply(vsg::ButtonReleaseEvent& buttonRelease) override;
    virtual void apply(vsg::MoveEvent& moveEvent) override;
    virtual void apply(vsg::ScrollWheelEvent& scrollWheel) override;

    virtual void apply(vsg::FrameEvent& frame) override;

    const vsg::dvec3& get_front() const;
    const vsg::dvec3& get_right() const;
    const vsg::dvec3& get_up() const;

    /// Сохранить route1.map (с бэкапом .prev); вызывается по Ctrl+S
    /// и из меню GUI
    void save_route() const;

private:
    void handle_shortcuts(vsg::KeyPressEvent& keyPress);

    /// Захват следующей клавиши для назначения (окно «Клавиши»)
    void capture_key_binding(vsg::KeyPressEvent& keyPress);

    /// Фокус камеры на выделении (клавиша F, промт п.47)
    void focus_on_selection();

    /// Переключение гизмо по циклу translate -> rotate -> scale -> off
    void cycle_gizmo();

    /// Прямой выбор режима гизмо (клавиши по таблице привязок)
    void set_gizmo_mode(GizmoMode mode);

    /// Начать отслеживание рамки выделения (зажатие ЛКМ)
    void start_selection_rect(const vsg::ButtonPressEvent& buttonPress);

    /// Завершить рамку выделения (отпускание ЛКМ)
    void finish_selection_rect();

    /// Выделить объекты, чьи центры попали в рамку
    void select_objects_in_rect(bool ctrl_add);

    void update_camera_vectors();

    void move_camera();

private:
    EditorContext& context_;

    vsg::ref_ptr<Keyboard> keyboard;

    bool is_lmb_pressed_ = false;
    bool is_mmb_pressed_ = false;
    bool is_rmb_pressed_ = false;

    /// Ctrl был зажат в момент начала рамки выделения (добавление)
    bool rect_ctrl_add_ = false;

    vsg::ivec2 mouse_pos_ = {0, 0};
    vsg::ivec2 delta_mouse_pos_ = {0, 0};
    bool has_prev_mouse_pos_ = false;

    double yaw_deg_ = 0.0;
    double pitch_deg_ = 0.0;

    vsg::dvec3 front_ = {0.0, 1.0, 0.0};
    vsg::dvec3 right_ = {1.0, 0.0, 0.0};
    vsg::dvec3 up_ = {0.0, 0.0, 1.0};
};

#endif // EDITOR_EVENT_HANDLER_H
