#ifndef ROUTE_EDITOR_H
#define ROUTE_EDITOR_H

#include "editor/settings/CameraSettings.h"
#include "editor/settings/GuiSettings.h"
#include "editor/settings/SceneSettings.h"
#include "editor/settings/WindowSettings.h"

#include <vsg/core/ref_ptr.h>

struct EditorContext;
class Camera;
class EditorGui;
class EventHandler;

namespace vsg
{

class ClearAttachments;
class CommandGraph;
class Group;
class Options;
class RenderGraph;
class ResourceHints;
class View;
class Viewer;
class Window;

}

namespace vsgImGui
{

class RenderImGui;

}

class RouteEditor
{
public:
    /**
     * @brief Construct a new RouteEditor object.
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     */
    RouteEditor();

    /**
     * @brief Destroy the RouteEditor object.
     */
    ~RouteEditor();

    /**
     * @brief Run event loop.
     *
     * Handle events, update, render.
     */
    void run();

private:
    window_settings_t window_settings;
    camera_settings_t camera_settings;
    scene_settings_t scene_settings;
    gui_settings_t gui_settings;

    EditorContext* context = nullptr;

    vsg::ref_ptr<vsg::Options> vsg_options;
    vsg::ref_ptr<vsg::Window> window;
    vsg::ref_ptr<EventHandler> event_handler;
    vsg::ref_ptr<Camera> camera;
    vsg::ref_ptr<vsg::Group> scenegraph;
    vsg::ref_ptr<vsg::View> scene_view;
    vsg::ref_ptr<vsg::ClearAttachments> clear_attachments;
    vsg::ref_ptr<EditorGui> editor_gui;
    vsg::ref_ptr<vsgImGui::RenderImGui> render_gui;
    vsg::ref_ptr<vsg::RenderGraph> render_graph;
    vsg::ref_ptr<vsg::CommandGraph> command_graph;
    vsg::ref_ptr<vsg::ResourceHints> resource_hints;
    vsg::ref_ptr<vsg::Viewer> viewer;

    double prev_frame_time = 0.0;

private:
    /**
     * @brief Initialize Journal subsystem.
     */
    void initialize_journal(const char* filename = "editor.log") const;

    /**
     * @brief Read the editor settings.
     */
    void read_settings(const char* filename = "editor-settings.xml");

    void print_settings() const;

    /**
     * @brief Create a VSG options object (paths, vsgXchange, shader sets).
     */
    void create_vsg_options();

    /**
     * @brief Configure shader sets for gltf models
     *        (flat/pbr/phong, как в старом редакторе).
     */
    void configure_shaders();

    /**
     * @brief Create a VSG window based on the window_settings.
     */
    void create_window();

    /**
     * @brief Create a scenegraph with ambient light.
     */
    void create_scenegraph();

    /**
     * @brief Create a scene view object.
     */
    void create_scene_view();

    /**
     * @brief Create editor handlers (events, intersections, selection).
     */
    void create_handlers();

    /**
     * @brief Create a render graph object.
     */
    void create_render_graph();

    /**
     * @brief Create a command graph object.
     */
    void create_command_graph();

    /**
     * @brief Create a resource hints object.
     */
    void create_resource_hints();

    /**
     * @brief Create a viewer object.
     */
    void create_viewer();

    /// Загрузить маршрут в сцену (состояние LOAD_ROUTE)
    void load_route();

    /// Скомпилировать узлы, добавленные в сцену с прошлого кадра
    void compile_models();
};

#endif // ROUTE_EDITOR_H
