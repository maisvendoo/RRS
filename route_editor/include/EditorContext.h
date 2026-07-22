#ifndef EDITOR_CONTEXT_H
#define EDITOR_CONTEXT_H

#include "MutexedVector.h"
#include "commands/CommandList.h"
#include "EditorState.h"
#include "RouteMap.h"
#include "RouteObject.h"

#include <atomic>
#include <mutex>
#include <thread>

#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>

#include <map>
#include <memory>
#include <string>

class Camera;
class Gizmo;
class IntersectionHandler;
class ObjectSelector;
class OutlineBuilder;
class Route;
class SceneGraph;
class Topology;
class WindowHandler;

namespace vsg
{

class Options;
class PagedLOD;
class Window;

}

struct ObjectRef
{
    std::string relative_path;
    vsg::ref_ptr<vsg::PagedLOD> paged_lod;
};

struct CompileInfo
{
    vsg::ref_ptr<vsg::Node> group_node;
    vsg::ref_ptr<vsg::Node> node;
    vsg::Mask mask = vsg::MASK_OFF;
};

struct WaypointData
{
    std::string trajectory_name;
    int direction;
    double coord;
    double length;
};

struct EditorContext
{
    EditorContext();
    ~EditorContext();

    EditorState state = EditorState::SELECT_ROUTE;
    CommandList commands;

    vsg::ref_ptr<vsg::Options> options;

    vsg::ref_ptr<vsg::Window> window;

    vsg::ref_ptr<IntersectionHandler> intersection_handler;

    vsg::ref_ptr<SceneGraph> scene_graph;
    vsg::ref_ptr<Route> route;

    RouteObjects static_objects;
    std::mutex static_objects_mutex;
    std::atomic_size_t static_objects_count = 0;
    std::atomic_size_t total_static_objects_count = 0;

    RouteObjects selected_objects;
    RouteObjects copied_objects;
    RouteObjects hidden_objects;

    MutexedVector<CompileInfo> compile_infos;

    std::unique_ptr<Topology> topology;
    std::mutex topology_mutex;
    std::atomic_bool topology_loaded = false;
    std::atomic_size_t topology_objects_count = 0;
    std::atomic_size_t total_topology_objects_count = 0;

    std::thread load_static_objects_thread;
    std::thread load_topology_thread;

    vsg::ref_ptr<Gizmo> gizmo;
    vsg::ref_ptr<ObjectSelector> object_selector;
    vsg::ref_ptr<OutlineBuilder> outline_builder;

    std::map<std::string, ObjectRef> objects_ref;
    RouteMap route_map;
    std::map<std::string, vsg::dvec3> stations_conf;
    std::map<std::string, WaypointData> waypoints_conf;

    std::string route_dir;

    double delta_time = 0.0;

    RouteObjects deferred_selection;
};

#endif // EDITOR_CONTEXT_H
