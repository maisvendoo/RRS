#ifndef EDITOR_CONTEXT_H
#define EDITOR_CONTEXT_H

#include "editor/MutexedVector.h"
#include "editor/RouteMap.h"
#include "editor/RouteObject.h"
#include "editor/commands/CommandList.h"
#include "editor/states/EditorState.h"

#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

class Camera;
class EventHandler;
class IntersectionHandler;
class ObjectSelector;
class Route;
class Topology;
struct camera_settings_t;
struct gui_settings_t;

namespace vsg
{

class Group;
class Options;
class PagedLOD;
class Viewer;

}

/// Ссылка на модель из objects.ref
struct ObjectRef
{
    std::string relative_path;
    vsg::ref_ptr<vsg::PagedLOD> paged_lod;
};

/// Информация о компиляции нового узла сцены в рендерер
struct CompileInfo
{
    vsg::ref_ptr<vsg::Node> group_node;
    vsg::ref_ptr<vsg::Node> node;
    vsg::Mask mask = vsg::MASK_OFF;
};

/// Данные точки из waypoints.conf
struct WaypointData
{
    std::string trajectory_name;
    int direction = 0;
    double coord = 0.0;
    double length = 0.0;
};

/// Общее состояние редактора: настройки, сцена, маршрут, команды
struct EditorContext
{
    EditorContext(camera_settings_t& camera_settings_,
                  gui_settings_t& gui_settings_);
    ~EditorContext();

    EditorContext(const EditorContext&) = delete;
    EditorContext& operator=(const EditorContext&) = delete;

    /// Текущее состояние редактора
    EditorState state = EditorState::NO_ROUTE;

    camera_settings_t& camera_settings;
    gui_settings_t& gui_settings;

    /// Список команд для undo/redo
    CommandList commands;

    vsg::ref_ptr<vsg::Options> options;
    vsg::ref_ptr<vsg::Group> scenegraph;
    vsg::ref_ptr<Camera> camera;
    vsg::ref_ptr<vsg::Viewer> viewer;

    vsg::ref_ptr<EventHandler> event_handler;
    vsg::ref_ptr<IntersectionHandler> intersection_handler;
    vsg::ref_ptr<ObjectSelector> object_selector;

    /// Статические объекты маршрута (строки route1.map)
    RouteObjects static_objects;
    std::mutex static_objects_mutex;
    std::atomic_size_t static_objects_count = 0;
    std::atomic_size_t total_static_objects_count = 0;

    /// Выделенные объекты
    RouteObjects selected_objects;

    /// Очередь узлов на компиляцию (наполняется из фоновых потоков)
    MutexedVector<CompileInfo> compile_infos;

    /// Топология маршрута (загружается в фоновом потоке)
    std::unique_ptr<Topology> topology;
    std::mutex topology_mutex;
    std::atomic_bool topology_loaded = false;
    std::atomic_size_t topology_objects_count = 0;
    std::atomic_size_t total_topology_objects_count = 0;

    std::thread load_static_objects_thread;
    std::thread load_topology_thread;

    std::map<std::string, ObjectRef> objects_ref;
    RouteMap route_map;
    std::map<std::string, vsg::dvec3> stations_conf;
    std::map<std::string, WaypointData> waypoints_conf;

    vsg::ref_ptr<Route> route;

    /// Каталог загруженного маршрута
    std::string route_dir;

    /// Время предыдущего кадра, с
    double delta_time = 0.0;

    /// Текст строки состояния (обновляется GUI и обработчиками)
    std::string status;

    /// Нажата ли Shift (обновляется EventHandler, используется выделением)
    bool shift_pressed = false;
};

#endif // EDITOR_CONTEXT_H
