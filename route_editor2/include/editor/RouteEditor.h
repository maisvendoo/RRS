#ifndef ROUTE_EDITOR_H
#define ROUTE_EDITOR_H

#include "editor/KeyBindings.h"
#include "editor/Route.h"
#include "editor/StateManager.h"
#include "editor/settings/CameraSettings.h"
#include "editor/settings/GuiSettings.h"
#include "editor/settings/SceneSettings.h"
#include "editor/settings/WindowSettings.h"

#include <vsg/core/ref_ptr.h>

#include <memory>
#include <string>

class Camera;
class EditorGui;
class EventHandler;
class Keyboard;
class Mouse;
class ObjectManager;

namespace vsg
{

class ClearAttachments;
class CloseHandler;
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
    KeyBindings key_bindings;

    StateManager state_manager;
    std::string route_dir;
    Route route;
    std::unique_ptr<ObjectManager> object_manager;

    vsg::ref_ptr<vsg::Options> vsg_options;
    vsg::ref_ptr<vsg::Window> window;
    vsg::ref_ptr<vsg::CloseHandler> close_handler;
    vsg::ref_ptr<Mouse> mouse;
    vsg::ref_ptr<Keyboard> keyboard;
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

private:
    /**
     * @brief Initialize Journal subsystem.
     *
     * Add additional storage for the editor to Journal
     * subsystem in the default logs directory with the
     * specified filename and start writing to it.
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check stderr
     * (standard error output stream) for possible errors).
     *
     * @param[in] filename Log filename (default - "editor.log").
     */
    void initialize_journal(const char* filename = "editor.log") const;

    /**
     * @brief Read the editor settings.
     *
     * Create a CfgReader and read the editor settings from the config file
     * in the default configs directory with the specified filename.
     *
     * If an error occured, it usually means that CfgReader was
     * unable to load the config file. Check the log file
     * (default - "logs/editor.log") for possible errors.
     *
     * @param[in] filename Config filename (default = "editor-settings.xml").
     */
    void read_settings(const char* filename = "editor-settings.xml");

    void print_settings() const;

    void create_object_manager();

    /**
     * @brief Create a VSG options object.
     *
     * Create a default VSG options object (with configured paths,
     * vsg::SharedObjects and vsgXchange included).
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     */
    void create_vsg_options();

    void configure_shaders();

    /**
     * @brief Create an EventHandler object.
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     */
    void create_event_handler();

    /**
     * @brief Create a Camera object.
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     */
    void create_camera();

    /**
     * @brief Create a VSG window based on the window_settings.
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     */
    void create_window();

    void create_mouse();

    /**
     * @brief Create a Keyboard object.
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     */
    void create_keyboard();

    /**
     * @brief Create a scenegraph object.
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     */
    void create_scenegraph();

    /**
     * @brief Create a scene view object.
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     */
    void create_scene_view();

    /**
     * @brief Create a ClearAttachments object.
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     */
    void create_clear_attachments();

    /**
     * @brief Create an EditorGui object.
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     */
    void create_editor_gui();

    /**
     * @brief Create a RenderGui object.
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     */
    void create_render_gui();

    /**
     * @brief Create a RenderGraph object.
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     */
    void create_render_graph();

    /**
     * @brief Create a CommandGraph object.
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     */
    void create_command_graph();

    /**
     * @brief Create a ResourceHints object.
     *
     * Set numLightsRange based on scene settings.
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     */
    void create_resource_hints();

    /**
     * @brief Create a Viewer object.
     *
     * Assign window and all appropriate event handlers to viewer.
     *
     * Compile viewer with resource hints.
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     */
    void create_viewer();
};

#endif // ROUTE_EDITOR_H
