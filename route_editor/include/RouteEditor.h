#ifndef ROUTE_EDITOR_H
#define ROUTE_EDITOR_H

#include "EditorContext.h"
#include "EditorState.h"
#include "KeyBindings.h"
#include "ObjectManager.h"
#include "commands/CommandManager.h"
#include "settings/CameraSettings.h"
#include "settings/GizmoSettings.h"
#include "settings/GuiSettings.h"
#include "settings/SceneSettings.h"
#include "settings/WindowSettings.h"

#include <vsg/io/Options.h>

#include <memory>

class Camera;
class Gizmo;
class Keyboard;
class Mouse;
class Route;
class SceneGraph;
class StateManager;
class WindowHandler;

class RouteEditor
{
public:
    RouteEditor();

    ~RouteEditor();

    bool initialize();

    void run();

private:
    void initialize_journal(const char* filename = "editor.log") const;

    void read_settings();

    void create_vsg_options();

    void configure_shaders();

    void compile_models();

    void handle_deferred_selection();

private:
    EditorContext context_;
    vsg::ref_ptr<vsg::Viewer> viewer_;
    vsg::ref_ptr<WindowHandler> window_handler_;
    vsg::ref_ptr<Mouse> mouse;
    vsg::ref_ptr<Keyboard> keyboard;
    std::unique_ptr<StateManager> state_manager;
    camera_settings_t camera_settings;
    gizmo_settings_t gizmo_settings;
    gui_settings_t gui_settings;
    scene_settings_t scene_settings;
    window_settings_t window_settings;
    KeyBindings key_bindings;
    vsg::ref_ptr<Camera> camera;
    EditorState editor_state = EditorState::SELECT_ROUTE;
    CommandManager command_manager;
    vsg::ref_ptr<vsg::Options> vsg_options;
    vsg::ref_ptr<vsg::Window> window;
    vsg::ref_ptr<SceneGraph> scene_graph;
    vsg::ref_ptr<Route> route;
    std::string route_dir;
    vsg::ref_ptr<Gizmo> gizmo;
    std::unique_ptr<ObjectManager> object_manager;
};

#endif // ROUTE_EDITOR_H
