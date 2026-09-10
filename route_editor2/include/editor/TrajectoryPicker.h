#ifndef EDITOR_TRAJECTORY_PICKER_H
#define EDITOR_TRAJECTORY_PICKER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

struct EditorContext;
class Trajectory;

namespace vsg
{

class ButtonPressEvent;

}

/**
 * @brief Выбор траектории кликом ЛКМ в режиме "Пути" (клавиша P).
 *
 * Луч камеры берётся из интерсектора IntersectionHandler, созданного
 * по нажатию ЛКМ. Для каждой траектории точки полилинии берутся через
 * Trajectory::getPosition каждые ~10 м, выбирается траектория с
 * минимальным расстоянием луч-отрезок.
 */
class TrajectoryPicker : public vsg::Inherit<vsg::Visitor, TrajectoryPicker>
{
public:
    explicit TrajectoryPicker(EditorContext& context);

    void apply(vsg::ButtonPressEvent& buttonPress) override;

private:
    /// Поиск ближайшей к лучу курсора траектории
    /// (nullptr - все траектории дальше порога захвата).
    /// Вызывается под topology_mutex.
    Trajectory* pick_closest_trajectory(const vsg::dvec3& ray_begin,
        const vsg::dvec3& ray_end) const;

private:
    EditorContext& context_;
};

#endif // EDITOR_TRAJECTORY_PICKER_H
