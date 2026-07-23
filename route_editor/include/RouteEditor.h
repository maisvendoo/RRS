#ifndef ROUTE_EDITOR_H
#define ROUTE_EDITOR_H

#include "EditorContext.h"
#include "EditorState.h"
#include "KeyBindings.h"
#include "commands/CommandList.h"
#include "settings/CameraSettings.h"
#include "settings/GizmoSettings.h"
#include "settings/GuiSettings.h"
#include "settings/SceneSettings.h"
#include "settings/WindowSettings.h"

#include <memory>

class Keyboard;
class Mouse;
class StateManager;

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
    CommandList command_list;
};

#endif // ROUTE_EDITOR_H
