#ifndef ROUTE_EDITOR_H
#define ROUTE_EDITOR_H

#include "editor/settings/CameraSettings.h"
#include "editor/settings/WindowSettings.h"

#include <vsg/core/ref_ptr.h>

class Camera;

namespace vsg
{

class Options;
class Window;

}

class RouteEditor
{
public:
    /**
     * @brief Construct a new RouteEditor object.
     *
     * Construct a new RouteEditor object and set param success
     * depending on the success of the initialization.
     * If an error occured during initialization, check the log
     * file (default - "logs/editor.log") for possible errors.
     *
     * @param[out] success true if initialization was successful,
     *                     false - otherwise.
     */
    explicit RouteEditor(bool& success);

    /**
     * @brief Destroy the RouteEditor object.
     */
    ~RouteEditor();

private:
    window_settings_t window_settings;
    camera_settings_t camera_settings;

    vsg::ref_ptr<vsg::Options> vsg_options;
    vsg::ref_ptr<vsg::Window> window;
    vsg::ref_ptr<Camera> camera;

private:
    /**
     * @brief Initialize Journal subsystem.
     *
     * Add additional storage for the editor to Journal
     * subsystem in the default logs directory with the
     * specified filename and start writing to it.
     *
     * @param[in] filename Log filename (default - "editor.log").
     */
    void initialize_journal(const char* filename = "editor.log") const;

    /**
     * @brief Read the editor settings.
     *
     * Create a CfgReader and read the editor settings from the config file
     * in the default configs directory with the specified filename.
     * If an error occured, it usually means that CfgReader was
     * unable to load the config file. Check the log file
     * (default - "logs/editor.log") for possible errors.
     *
     * @param[in] filename Config filename (default = "editor-settings.xml").
     * @return true if the editor settings were read successfully.
     * @return false - otherwise.
     */
    bool read_settings(const char* filename = "editor-settings.xml");

    /**
     * @brief Create a VSG window based on the window_settings.
     *
     * @return true if the window was created successfully.
     * @return false - otherwise.
     */
    bool create_window();
};

#endif // ROUTE_EDITOR_H
