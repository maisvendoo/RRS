#include "editor/Gizmo.h"

#include "editor/Camera.h"
#include "editor/EditorContext.h"
#include "editor/EventHandler.h"
#include "editor/IntersectionHandler.h"
#include "editor/Mask.h"
#include "editor/MouseButton.h"
#include "editor/RouteObject.h"
#include "editor/commands/CommandList.h"
#include "editor/commands/RotateObjects.h"
#include "editor/commands/ScaleObjects.h"
#include "editor/commands/TranslateObjects.h"
#include "editor/states/EditorState.h"

#include <vsgImGui/imgui.h>

#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/box.h>
#include <vsg/maths/common.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/utils/Builder.h>
#include <vsg/utils/ShaderSet.h>

#include <algorithm>
#include <cmath>

static constexpr vsg::vec3 X_AXIS_POSITIVEf = {1.0f, 0.0f, 0.0f};
static constexpr vsg::vec3 Y_AXIS_POSITIVEf = {0.0f, 1.0f, 0.0f};
static constexpr vsg::vec3 Z_AXIS_POSITIVEf = {0.0f, 0.0f, 1.0f};

static constexpr vsg::dvec3 X_AXIS_POSITIVEd = {1.0, 0.0, 0.0};
static constexpr vsg::dvec3 Y_AXIS_POSITIVEd = {0.0, 1.0, 0.0};
static constexpr vsg::dvec3 Z_AXIS_POSITIVEd = {0.0, 0.0, 1.0};

/// Округление вектора к шагу сетки (привязка, клавиша X)
static vsg::dvec3 snap_to_grid(const vsg::dvec3& value, double step)
{
    if (step <= 0.0)
    {
        return value;
    }

    return vsg::dvec3{
        std::round(value.x / step) * step,
        std::round(value.y / step) * step,
        std::round(value.z / step) * step};
}

/// Поворот геометрии по умолчанию (вдоль +Z) к заданному направлению
static vsg::mat4 rotate_to_direction(vsg::vec3 direction)
{
    const vsg::vec3 axis = vsg::cross(Z_AXIS_POSITIVEf, direction);

    if (vsg::length(axis) < 1.0e-6f)
    {
        // Направление совпадает с +Z или противоположно ему
        if (vsg::dot(Z_AXIS_POSITIVEf, direction) < 0.0f)
        {
            return vsg::rotate(static_cast<float>(vsg::PI),
                X_AXIS_POSITIVEf);
        }

        return vsg::mat4();
    }

    const float angle = std::acos(vsg::dot(Z_AXIS_POSITIVEf, direction));

    return vsg::rotate(angle, vsg::normalize(axis));
}

Gizmo::Gizmo(EditorContext& context, const gizmo_settings_t& gizmo_settings_)
    : context_(context)
    , gizmo_settings(gizmo_settings_)
{
    // Шейдеры гизмо - настроенный flat-набор сцены
    const auto shader_it = context_.options->shaderSets.find("flat");

    if (shader_it != context_.options->shaderSets.end())
    {
        builder_.shaderSet = shader_it->second;
    }
    else
    {
        builder_.shaderSet = vsg::createFlatShadedShaderSet();
    }

    const vsg::vec3 axis_colors[] = {
        gizmo_settings.axis_x_color,
        gizmo_settings.axis_y_color,
        gizmo_settings.axis_z_color
    };

    const float plane_width = 1.0e6f;
    const float line_thickness = 0.01f;

    vsg::StateInfo state_info;
    state_info.two_sided = true;
    state_info.blending = true;

    // Стрелка перемещения: цилиндр + конус вдоль направления
    const auto create_arrow = [&](vsg::vec3 direction,
        vsg::vec3 color) -> vsg::ref_ptr<vsg::Node>
    {
        float thickness = gizmo_settings.arrow_thickness;
        const float length = gizmo_settings.arrow_length;
        const float opacity = gizmo_settings.opacity;

        vsg::box box = {
            vsg::vec3(-thickness, -thickness, 0.0f),
            vsg::vec3( thickness,  thickness, length)
        };

        vsg::GeometryInfo geometry_info(box);
        geometry_info.transform = rotate_to_direction(direction);
        geometry_info.color = {color, opacity};

        const auto cylinder = builder_.createCylinder(geometry_info,
            state_info);

        thickness *= 3.0f;

        box.min = {-thickness, -thickness, length};
        box.max = { thickness,  thickness, length + thickness * 5.0f};

        geometry_info.set(box);

        const auto cone = builder_.createCone(geometry_info, state_info);

        const auto arrow = vsg::Group::create();
        arrow->addChild(cylinder);
        arrow->addChild(cone);

        return arrow;
    };

    // Ось масштабирования: тонкий цилиндр + кубик на конце
    const auto create_scale_axis = [&](vsg::vec3 direction,
        vsg::vec3 color) -> vsg::ref_ptr<vsg::Node>
    {
        float thickness = gizmo_settings.arrow_thickness * 2.0f;
        const float length = gizmo_settings.arrow_length;
        const float opacity = gizmo_settings.opacity;

        vsg::box box = {
            vsg::vec3(-thickness, -thickness, 0.0f),
            vsg::vec3( thickness,  thickness, length)
        };

        vsg::GeometryInfo geometry_info(box);
        geometry_info.transform = rotate_to_direction(direction);
        geometry_info.color = {color, opacity};

        const auto cylinder = builder_.createCylinder(geometry_info,
            state_info);

        thickness *= 3.0f;

        box.min = {-thickness, -thickness, length};
        box.max = { thickness,  thickness, length + thickness * 4.0f};

        geometry_info.set(box);

        const auto tip = builder_.createBox(geometry_info, state_info);

        const auto axis = vsg::Group::create();
        axis->addChild(cylinder);
        axis->addChild(tip);

        return axis;
    };

    // Кольцо поворота: кольцо из тонких сегментов в плоскости XY,
    // затем поворот плоскости перпендикулярно заданной оси
    const auto create_ring = [&](vsg::vec3 normal,
        vsg::vec3 color) -> vsg::ref_ptr<vsg::Node>
    {
        const float radius = gizmo_settings.arrow_length;
        const float thickness = gizmo_settings.arrow_thickness * 3.0f;
        const float opacity = gizmo_settings.opacity;

        constexpr unsigned int segments = 48;
        const float segment_length = 2.0f * radius *
            std::sin(static_cast<float>(vsg::PI) /
                static_cast<float>(segments));

        const auto ring = vsg::Group::create();

        for (unsigned int i = 0; i < segments; ++i)
        {
            const float angle = 2.0f * static_cast<float>(vsg::PI) *
                static_cast<float>(i) / static_cast<float>(segments);

            const vsg::vec3 middle = {
                radius * std::cos(angle),
                radius * std::sin(angle),
                0.0f};

            const vsg::vec3 tangent = {
                -std::sin(angle),
                std::cos(angle),
                0.0f};

            const vsg::box box = {
                vsg::vec3(-thickness, -thickness, -segment_length * 0.5f),
                vsg::vec3( thickness,  thickness,  segment_length * 0.5f)
            };

            vsg::GeometryInfo geometry_info(box);
            geometry_info.color = {color, opacity};
            geometry_info.transform = vsg::translate(middle) *
                rotate_to_direction(tangent);

            ring->addChild(builder_.createCylinder(geometry_info, state_info));
        }

        const auto matrix_transform = vsg::MatrixTransform::create();
        matrix_transform->matrix = rotate_to_direction(normal);
        matrix_transform->addChild(ring);

        return matrix_transform;
    };

    // Плоскость-приёмник перетаскивания (нормаль задаёт плоскость)
    const auto create_plane = [&](vsg::vec3 normal) -> vsg::ref_ptr<vsg::Node>
    {
        const float width = plane_width;

        const vsg::box box = {
            vsg::vec3(-width, -width, 0.0f),
            vsg::vec3( width,  width, 0.0f)
        };

        vsg::GeometryInfo geometry_info(box);
        geometry_info.transform = rotate_to_direction(normal);

        return builder_.createQuad(geometry_info, state_info);
    };

    // Линия-подсветка активной оси на время перетаскивания
    const auto create_line = [&](vsg::vec3 direction,
        vsg::vec3 color) -> vsg::ref_ptr<vsg::Node>
    {
        const float width = plane_width;
        const float thickness = line_thickness;
        const float opacity = gizmo_settings.opacity;

        const vsg::box box = {
            vsg::vec3(-thickness, -thickness, -width),
            vsg::vec3( thickness,  thickness,  width)
        };

        vsg::GeometryInfo geometry_info(box);
        geometry_info.transform = rotate_to_direction(direction);
        geometry_info.color = {color, opacity};

        return builder_.createCylinder(geometry_info, state_info);
    };

    // Ветвь перемещения
    const auto translate_group = vsg::Group::create();
    arrow_x_ = create_arrow(X_AXIS_POSITIVEf, axis_colors[0]);
    arrow_y_ = create_arrow(Y_AXIS_POSITIVEf, axis_colors[1]);
    arrow_z_ = create_arrow(Z_AXIS_POSITIVEf, axis_colors[2]);
    translate_group->addChild(arrow_x_);
    translate_group->addChild(arrow_y_);
    translate_group->addChild(arrow_z_);
    translate_switch_ = SingleSwitch::create(vsg::MASK_OFF, translate_group);

    // Ветвь поворота
    const auto rotate_group = vsg::Group::create();
    ring_x_ = create_ring(X_AXIS_POSITIVEf, axis_colors[0]);
    ring_y_ = create_ring(Y_AXIS_POSITIVEf, axis_colors[1]);
    ring_z_ = create_ring(Z_AXIS_POSITIVEf, axis_colors[2]);
    rotate_group->addChild(ring_x_);
    rotate_group->addChild(ring_y_);
    rotate_group->addChild(ring_z_);
    rotate_switch_ = SingleSwitch::create(vsg::MASK_OFF, rotate_group);

    // Ветвь масштабирования
    const auto scale_group = vsg::Group::create();
    scale_x_ = create_scale_axis(X_AXIS_POSITIVEf, axis_colors[0]);
    scale_y_ = create_scale_axis(Y_AXIS_POSITIVEf, axis_colors[1]);
    scale_z_ = create_scale_axis(Z_AXIS_POSITIVEf, axis_colors[2]);
    scale_group->addChild(scale_x_);
    scale_group->addChild(scale_y_);
    scale_group->addChild(scale_z_);
    scale_switch_ = SingleSwitch::create(vsg::MASK_OFF, scale_group);

    // Плоскости и линии перетаскивания (включаются на время драга)
    plane_yz_switch_ = SingleSwitch::create(vsg::MASK_OFF,
        create_plane(X_AXIS_POSITIVEf));
    plane_xz_switch_ = SingleSwitch::create(vsg::MASK_OFF,
        create_plane(Y_AXIS_POSITIVEf));
    plane_xy_switch_ = SingleSwitch::create(vsg::MASK_OFF,
        create_plane(Z_AXIS_POSITIVEf));

    line_x_switch_ = SingleSwitch::create(vsg::MASK_OFF,
        create_line(X_AXIS_POSITIVEf, axis_colors[0]));
    line_y_switch_ = SingleSwitch::create(vsg::MASK_OFF,
        create_line(Y_AXIS_POSITIVEf, axis_colors[1]));
    line_z_switch_ = SingleSwitch::create(vsg::MASK_OFF,
        create_line(Z_AXIS_POSITIVEf, axis_colors[2]));

    matrix_transform_ = vsg::MatrixTransform::create();
    matrix_transform_->addChild(translate_switch_);
    matrix_transform_->addChild(rotate_switch_);
    matrix_transform_->addChild(scale_switch_);
    matrix_transform_->addChild(plane_yz_switch_);
    matrix_transform_->addChild(plane_xz_switch_);
    matrix_transform_->addChild(plane_xy_switch_);
    matrix_transform_->addChild(line_x_switch_);
    matrix_transform_->addChild(line_y_switch_);
    matrix_transform_->addChild(line_z_switch_);

    this->node = matrix_transform_;

    update();
}

bool Gizmo::handle_press(const vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled || dragging_)
    {
        return false;
    }

    if (mode_ == GizmoMode::OFF ||
        context_.selected_objects.empty() ||
        context_.trajectory_mode ||
        context_.state != EditorState::EDIT_ROUTE)
    {
        return false;
    }

    // Клик по окну ImGui не должен захватывать гизмо
    if (ImGui::GetCurrentContext() != nullptr &&
        ImGui::GetIO().WantCaptureMouse)
    {
        return false;
    }

    const LSIntersectorRefPtr intersector =
        context_.intersection_handler->get_lmb_intersector();

    if (!intersector)
    {
        return false;
    }

    // Пикинг гизмо собственным лучом (по образцу старого редактора)
    this->accept(*intersector);

    const LSIntersectionRefPtr intersection =
        IntersectionHandler::get_closest_intersection(intersector);

    if (!intersection)
    {
        intersector->intersections.clear();
        return false;
    }

    const vsg::dvec3& world_intersection = intersection->worldIntersection;

    // Направление взгляда камеры (для выбора плоскости перетаскивания)
    const vsg::dvec3& camera_front = context_.event_handler->get_front();

    const double dot_x = std::abs(vsg::dot(camera_front, X_AXIS_POSITIVEd));
    const double dot_y = std::abs(vsg::dot(camera_front, Y_AXIS_POSITIVEd));
    const double dot_z = std::abs(vsg::dot(camera_front, Z_AXIS_POSITIVEd));

    for (const vsg::Node* const node : intersection->nodePath)
    {
        // Определяем индекс оси, за которую ухватились
        int axis = -1;

        if (mode_ == GizmoMode::TRANSLATE)
        {
            if (node == arrow_x_.get())
            {
                axis = 0;
            }
            else if (node == arrow_y_.get())
            {
                axis = 1;
            }
            else if (node == arrow_z_.get())
            {
                axis = 2;
            }
        }
        else if (mode_ == GizmoMode::ROTATE)
        {
            if (node == ring_x_.get())
            {
                axis = 0;
            }
            else if (node == ring_y_.get())
            {
                axis = 1;
            }
            else if (node == ring_z_.get())
            {
                axis = 2;
            }
        }
        else if (mode_ == GizmoMode::SCALE)
        {
            if (node == scale_x_.get())
            {
                axis = 0;
            }
            else if (node == scale_y_.get())
            {
                axis = 1;
            }
            else if (node == scale_z_.get())
            {
                axis = 2;
            }
        }

        if (axis < 0)
        {
            continue;
        }

        dragging_ = true;
        drag_axis_index_ = axis;
        drag_axis_ = axis_vector(axis);
        drag_pivot_ = curr_pos_;
        drag_length_unit_ = static_cast<double>(
            gizmo_settings.arrow_length) * scale_;
        prev_intersect_pos_ = world_intersection;

        raw_total_translation_ = {0.0, 0.0, 0.0};
        applied_translation_ = {0.0, 0.0, 0.0};
        total_radians_ = 0.0;
        total_scale_ = 1.0;

        if (mode_ == GizmoMode::ROTATE)
        {
            // Вращение вокруг оси: плоскость, перпендикулярная оси
            active_plane_switch_ =
                (axis == 0) ? plane_yz_switch_ :
                (axis == 1) ? plane_xz_switch_ : plane_xy_switch_;

            active_line_switch_ = nullptr;
        }
        else
        {
            // Перемещение/масштаб вдоль оси: содержащая ось плоскость,
            // развернутая к камере (как в старом редакторе)
            if (axis == 0)
            {
                active_plane_switch_ = (dot_y > dot_z)
                    ? plane_xz_switch_
                    : plane_xy_switch_;
            }
            else if (axis == 1)
            {
                active_plane_switch_ = (dot_x > dot_z)
                    ? plane_yz_switch_
                    : plane_xy_switch_;
            }
            else
            {
                active_plane_switch_ = (dot_x > dot_y)
                    ? plane_yz_switch_
                    : plane_xz_switch_;
            }

            active_line_switch_ =
                (axis == 0) ? line_x_switch_ :
                (axis == 1) ? line_y_switch_ : line_z_switch_;
        }

        active_plane_switch_->mask = vsg::Mask{editor2::MASK_CLICKABLE};

        if (active_line_switch_)
        {
            active_line_switch_->mask = vsg::Mask{editor2::MASK_GUI1};
        }

        for (const auto& object : context_.selected_objects)
        {
            object->save_matrix();
        }

        break;
    }

    intersector->intersections.clear();

    return dragging_;
}

void Gizmo::handle_release(const vsg::ButtonReleaseEvent& buttonRelease)
{
    if (!dragging_)
    {
        return;
    }

    if (buttonRelease.button != editor2::MOUSE_BUTTON_LEFT)
    {
        return;
    }

    const RouteObjects& selected_objects = context_.selected_objects;

    // Записываем undo-команду с накопленной трансформацией
    // (объекты уже перемещены, поэтому execute = false)
    switch (mode_)
    {
        case GizmoMode::TRANSLATE:
        {
            if (vsg::length(applied_translation_) > 1.0e-12)
            {
                context_.commands.push(new TranslateObjects(context_,
                    selected_objects, applied_translation_), false);

                context_.status = "Objects translated by gizmo";
            }

            break;
        }
        case GizmoMode::ROTATE:
        {
            if (std::abs(total_radians_) > 1.0e-9)
            {
                context_.commands.push(new RotateObjects(context_,
                    selected_objects, drag_pivot_, drag_axis_,
                    total_radians_), false);

                context_.status = "Objects rotated by gizmo";
            }

            break;
        }
        case GizmoMode::SCALE:
        {
            if (std::abs(total_scale_ - 1.0) > 1.0e-9)
            {
                context_.commands.push(new ScaleObjects(context_,
                    selected_objects, drag_pivot_,
                    vsg::dvec3{total_scale_, total_scale_, total_scale_}),
                    false);

                context_.status = "Objects scaled by gizmo";
            }

            break;
        }
        default:
        {
            break;
        }
    }

    dragging_ = false;
    drag_axis_index_ = -1;

    if (active_plane_switch_)
    {
        active_plane_switch_->mask = vsg::MASK_OFF;
        active_plane_switch_ = nullptr;
    }

    if (active_line_switch_)
    {
        active_line_switch_->mask = vsg::MASK_OFF;
        active_line_switch_ = nullptr;
    }
}

void Gizmo::handle_move(const vsg::MoveEvent& moveEvent)
{
    if (moveEvent.handled || !dragging_ || !active_plane_switch_)
    {
        return;
    }

    // Луч через текущую позицию курсора
    const auto intersector =
        context_.intersection_handler->apply_(moveEvent);

    intersector->traversalMask = editor2::MASK_CLICKABLE;

    active_plane_switch_->accept(*intersector);

    const LSIntersectionRefPtr intersection =
        IntersectionHandler::get_closest_intersection(intersector);

    if (!intersection)
    {
        return;
    }

    const vsg::dvec3& world_intersection = intersection->worldIntersection;

    if (mode_ == GizmoMode::TRANSLATE)
    {
        // Дельта вдоль выбранной оси (как в старом редакторе)
        vsg::dvec3 delta = {0.0, 0.0, 0.0};
        delta[drag_axis_index_] = world_intersection[drag_axis_index_] -
            prev_intersect_pos_[drag_axis_index_];

        prev_intersect_pos_ = world_intersection;
        raw_total_translation_ += delta;

        // Привязка к сетке (клавиша X): округляем накопленную дельту
        vsg::dvec3 total = raw_total_translation_;

        if (context_.snap_enabled)
        {
            total = snap_to_grid(total, context_.snap_step);
        }

        const vsg::dvec3 apply_delta = total - applied_translation_;

        if (vsg::length(apply_delta) > 1.0e-12)
        {
            applied_translation_ = total;

            for (const auto& object : context_.selected_objects)
            {
                object->move(apply_delta);
            }
        }

        return;
    }

    if (mode_ == GizmoMode::ROTATE)
    {
        // Угол между предыдущим и текущим радиус-векторами
        const vsg::dvec3 prev_vec = prev_intersect_pos_ - drag_pivot_;
        const vsg::dvec3 curr_vec = world_intersection - drag_pivot_;

        prev_intersect_pos_ = world_intersection;

        const double prev_length = vsg::length(prev_vec);
        const double curr_length = vsg::length(curr_vec);

        if (prev_length < 1.0e-9 || curr_length < 1.0e-9)
        {
            return;
        }

        const double cos_angle = std::clamp(
            vsg::dot(prev_vec, curr_vec) / (prev_length * curr_length),
            -1.0, 1.0);

        const double sin_angle = std::clamp(
            vsg::dot(vsg::cross(prev_vec, curr_vec), drag_axis_) /
                (prev_length * curr_length),
            -1.0, 1.0);

        const double delta_rad = std::atan2(sin_angle, cos_angle);

        if (std::abs(delta_rad) < 1.0e-9)
        {
            return;
        }

        total_radians_ += delta_rad;

        for (const auto& object : context_.selected_objects)
        {
            object->rotate_around_pivot(drag_pivot_, drag_axis_, delta_rad,
                object->matrix);
        }

        return;
    }

    if (mode_ == GizmoMode::SCALE)
    {
        // Равномерный масштаб пропорционально смещению вдоль оси
        const double delta_coord =
            world_intersection[drag_axis_index_] -
            prev_intersect_pos_[drag_axis_index_];

        prev_intersect_pos_ = world_intersection;

        if (drag_length_unit_ < 1.0e-6)
        {
            return;
        }

        double scale_value = 1.0 + delta_coord / drag_length_unit_;
        scale_value = std::clamp(scale_value, 0.01, 100.0);

        if (std::abs(scale_value - 1.0) < 1.0e-9)
        {
            return;
        }

        total_scale_ *= scale_value;

        const vsg::dvec3 scale{scale_value, scale_value, scale_value};

        for (const auto& object : context_.selected_objects)
        {
            object->scale_relative_to_pivot(drag_pivot_, scale,
                object->matrix);
        }
    }
}

void Gizmo::update()
{
    // Во время перетаскивания позиция гизмо фиксирована
    if (dragging_)
    {
        return;
    }

    const RouteObjects& selected_objects = context_.selected_objects;

    const bool visible = mode_ != GizmoMode::OFF &&
        !selected_objects.empty() && !context_.trajectory_mode;

    if (!visible)
    {
        this->mask = vsg::MASK_OFF;
        return;
    }

    this->mask = vsg::Mask{editor2::MASK_GUI1 | editor2::MASK_CLICKABLE};

    // Позиция: центр габарита выделения или средняя точка объектов
    curr_pos_ = {0.0, 0.0, 0.0};

    for (const auto& object : selected_objects)
    {
        if (gizmo_settings.to_center)
        {
            const vsg::dbox& bounds = object->get_bounds();

            if (bounds.valid())
            {
                curr_pos_ += (bounds.min + bounds.max) * 0.5;
                continue;
            }
        }

        curr_pos_ += object->get_translation();
    }

    curr_pos_ /= static_cast<double>(selected_objects.size());

    // Масштаб: гизмо занимает ~7.5% высоты кадра (как в старом редакторе)
    const vsg::ref_ptr<Camera> camera = context_.camera;

    if (!camera || !camera->get_perspective() || !camera->get_look_at())
    {
        return;
    }

    const double fov_rad =
        vsg::radians(camera->get_perspective()->fieldOfViewY);

    const double distance_to_camera =
        vsg::length(curr_pos_ - camera->get_look_at()->eye);

    scale_ = distance_to_camera * std::tan(fov_rad * 0.5) * 0.075;

    matrix_transform_->matrix =
        vsg::translate(curr_pos_) * vsg::scale(scale_);

    // Включаем только ветвь активного режима
    const vsg::Mask branch_mask =
        vsg::Mask{editor2::MASK_GUI1 | editor2::MASK_CLICKABLE};

    translate_switch_->mask =
        (mode_ == GizmoMode::TRANSLATE) ? branch_mask : vsg::MASK_OFF;

    rotate_switch_->mask =
        (mode_ == GizmoMode::ROTATE) ? branch_mask : vsg::MASK_OFF;

    scale_switch_->mask =
        (mode_ == GizmoMode::SCALE) ? branch_mask : vsg::MASK_OFF;
}

void Gizmo::set_mode(GizmoMode mode)
{
    // Смену режима в середине перетаскивания игнорируем
    if (dragging_)
    {
        return;
    }

    mode_ = mode;

    update();
}

GizmoMode Gizmo::get_mode() const
{
    return mode_;
}

GizmoMode Gizmo::next_mode(GizmoMode mode)
{
    switch (mode)
    {
        case GizmoMode::OFF:
        {
            return GizmoMode::TRANSLATE;
        }
        case GizmoMode::TRANSLATE:
        {
            return GizmoMode::ROTATE;
        }
        case GizmoMode::ROTATE:
        {
            return GizmoMode::SCALE;
        }
        default:
        {
            return GizmoMode::OFF;
        }
    }
}

const vsg::dvec3& Gizmo::get_curr_pos() const
{
    return curr_pos_;
}

vsg::dvec3 Gizmo::axis_vector(int index)
{
    switch (index)
    {
        case 0:
        {
            return X_AXIS_POSITIVEd;
        }
        case 1:
        {
            return Y_AXIS_POSITIVEd;
        }
        default:
        {
            return Z_AXIS_POSITIVEd;
        }
    }
}
