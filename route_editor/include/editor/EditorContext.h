#ifndef EDITOR_CONTEXT_H
#define EDITOR_CONTEXT_H

#include "editor/EditorState.h"
#include "editor/KeyBindings.h"
#include "editor/RouteMap.h"
#include "editor/RouteObjects.h"
#include "editor/settings/CameraSettings.h"
#include "editor/settings/GizmoSettings.h"
#include "editor/settings/GuiSettings.h"
#include "editor/settings/SceneSettings.h"
#include "editor/settings/WindowSettings.h"

#include <core/ThreadSafe.h>

#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class Camera;
class CommandManager;
class Gizmo;
class Keyboard;
class Mouse;
class ObjectManager;
class ObjectSelector;
class OutlineBuilder;
class Route;
class SceneGraph;
class StateManager;
class Topology;
class WorldCulling;

namespace vsg
{

class Node;
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

    window_settings_t window_settings;
    camera_settings_t camera_settings;
    scene_settings_t scene_settings;
    gizmo_settings_t gizmo_settings;
    gui_settings_t gui_settings;
    KeyBindings key_bindings;

    vsg::ref_ptr<Keyboard> keyboard;
    vsg::ref_ptr<Mouse> mouse;

    vsg::ref_ptr<vsg::Options> vsg_options;
    vsg::ref_ptr<vsg::Window> window;
    vsg::ref_ptr<Camera> camera;
    vsg::ref_ptr<vsg::Switch> scene_graph;
    vsg::ref_ptr<Route> route;
    vsg::ref_ptr<Gizmo> gizmo;
    std::unique_ptr<ObjectManager> object_manager;
    vsg::ref_ptr<WorldCulling> world_culling;

    EditorState editor_state = EditorState::SELECT_ROUTE;
    std::unique_ptr<StateManager> state_manager;
    std::unique_ptr<CommandManager> command_manager;

    ThreadSafe<RouteObjects> static_objects;

    RouteObjects selected_objects;
    RouteObjects copied_objects;
    RouteObjects hidden_objects;

    ThreadSafe<std::vector<CompileInfo>> compile_infos;

    ThreadSafe<std::unique_ptr<Topology>> topology;
    std::atomic_bool topology_loaded = false;

    std::thread load_static_objects_thread;
    std::thread load_topology_thread;

    vsg::ref_ptr<ObjectSelector> object_selector;
    vsg::ref_ptr<OutlineBuilder> outline_builder;

    std::map<std::string, ObjectRef> objects_ref;
    RouteMap route_map;
    std::map<std::string, vsg::dvec3> stations_conf;
    std::map<std::string, WaypointData> waypoints_conf;

    RouteObjects deferred_selection;

    std::atomic_bool finish_topology_thread;
};

#endif // EDITOR_CONTEXT_H
