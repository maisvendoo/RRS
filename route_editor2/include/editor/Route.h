#ifndef EDITOR_ROUTE_H
#define EDITOR_ROUTE_H

#include <vsg/core/Inherit.h>
#include <vsg/nodes/Switch.h>

#include <string>

struct EditorContext;
class Trajectory;

/// Загруженный маршрут: статические объекты + топология
/// Line-шейдеры (traj_line.vert/frag) для линий: экспорт для сетки/превью
vsg::ref_ptr<vsg::StateGroup> create_trajectory_lines_state_group(
    vsg::ref_ptr<const vsg::Options> options);

class Route : public vsg::Inherit<vsg::Switch, Route>
{
public:
    explicit Route(EditorContext& context);

    /// Выбрать траекторию по имени (пустое имя - снять выделение):
    /// обновляет EditorContext (имя + указатель) и подсветку линии
    void select_trajectory(const std::string& name);

    /// Превью полосы параметрической стройки (ТЗ п.6/55): оранжевая
    /// линия вдоль траектории в диапазоне [begin_m, end_m]
    void show_build_preview(const Trajectory* trajectory,
                            double begin_m, double end_m);

    /// Убрать превью стройки
    void hide_build_preview();

private:
    void load_geo_anchor();

    bool load_objects_ref();
    bool load_route_map();
    bool load_stations_conf();
    bool load_waypoints_conf();
    bool load_track_profiles();

    void load_static_objects();
    bool load_topology();

    /// Перестроить линию подсветки выбранной траектории (циан)
    void update_trajectory_highlight(const Trajectory* trajectory);

    /// Превью полосы параметрической стройки (ТЗ п.6/55): оранжевая

private:
    EditorContext& context_;
};

#endif // EDITOR_ROUTE_H
