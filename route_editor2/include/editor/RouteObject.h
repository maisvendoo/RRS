#ifndef EDITOR_ROUTE_OBJECT_H
#define EDITOR_ROUTE_OBJECT_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/box.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>

#include <list>
#include <string>

struct EditorContext;
#include <vsg/core/Mask.h>

class RouteObject;
class SingleSwitch;

namespace vsg
{

class PagedLOD;

}

using RouteObjects = std::list<vsg::ref_ptr<RouteObject>>;
using RouteObjectsIterator = RouteObjects::iterator;

/// Объект маршрута (строка route1.map): модель + трансформация
class RouteObject : public vsg::Inherit<vsg::MatrixTransform, RouteObject>
{
public:
    RouteObject(
        EditorContext& context,
        vsg::ref_ptr<vsg::PagedLOD> paged_lod,
        const std::string& label,
        const vsg::dvec3& translation,
        const vsg::dvec3& rotation_deg = {0.0, 0.0, 0.0},
        const vsg::dvec3& scale = {1.0, 1.0, 1.0}
    );

    const vsg::dvec3& get_translation() const;
    const vsg::dvec3& get_rotation_deg() const;
    const vsg::dvec3& get_scale() const;

    const vsg::dmat4& get_initial_matrix() const;
    const vsg::dbox& get_bounds() const;

    bool get_is_selected() const;

    void set_translation(const vsg::dvec3& translation);
    void set_rotation_deg(const vsg::dvec3& rotation_deg);
    void set_scale(const vsg::dvec3& scale);

    void move(const vsg::dvec3& translation);

    void rotate_around_pivot(const vsg::dvec3& pivot, const vsg::dvec3& axis,
        double radians, const vsg::dmat4& matrix);

    void scale_relative_to_pivot(const vsg::dvec3& pivot, const vsg::dvec3& scale,
        const vsg::dmat4& matrix);

    void select();
    RouteObjectsIterator deselect();

    vsg::ref_ptr<RouteObject> copy() const;

    void save_matrix();
    void set_matrix(const vsg::dmat4& matrix);

    /// Текущая модель объекта (подмена при смене метки, mass replace)
    vsg::ref_ptr<vsg::PagedLOD> get_paged_lod() const;

    /// Подменить модель объекта (окно «Mass edit»): заменяет дочерний
    /// PagedLOD и ставит узел на перекомпиляцию
    void replace_model(vsg::ref_ptr<vsg::PagedLOD> paged_lod);

public:
    /// Метка объекта (ключ из objects.ref)
    std::string label;

    /// Маска видимости узла (переключение видимости, слои)
    vsg::Mask mask = vsg::MASK_ALL;

    /// Слой объекта (окно «Слои»): "default" или заданный вручную;
    /// хранится в track-edit.conf, секция <Layer>
    std::string layer = "default";

private:
    void update_matrix();
    void update_bounds();
    void decompose_matrix();

private:
    EditorContext& context_;

    vsg::dvec3 translation_;
    vsg::dvec3 rotation_deg_;
    vsg::dvec3 scale_;

    vsg::dmat4 initial_matrix_;
    vsg::dbox bounds_;

    bool is_selected_ = false;

    vsg::ref_ptr<SingleSwitch> paged_lod_switch_;
    vsg::ref_ptr<vsg::PagedLOD> paged_lod_;
};

#endif // EDITOR_ROUTE_OBJECT_H
