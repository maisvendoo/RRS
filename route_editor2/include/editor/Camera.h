#ifndef EDITOR_CAMERA_H
#define EDITOR_CAMERA_H

#include <vsg/app/Camera.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>

#include <vulkan/vulkan_core.h>

class Keyboard;
class Mouse;
struct camera_settings_t;

namespace vsg
{

class LookAt;
class Orthographic;
class Perspective;

}

class Camera : public vsg::Inherit<vsg::Camera, Camera>
{
public:
    /**
     * @brief Construct a new Camera object.
     *
     * Create perspective and orthographic projection matrices,
     * LookAt view matrix and ViewportState based on the
     * camera_settings and window_extent parameters.
     *
     * Set camera's projection matrix to perspective mode.
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     *
     * @param[in] camera_settings The camera settings object.
     * @param[in] window_extent Window extent from the VSG window object.
     * @param[in] mouse The mouse object.
     * @param[in] keyboard The keyboard object.
     */
    Camera(
        const camera_settings_t& camera_settings,
        VkExtent2D window_extent,
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard
    );

    /**
     * @brief Destroy the Camera object.
     */
    ~Camera();

    void handle_mouse_move(int delta_x, int delta_y);

    void update(double delta_time);

    const vsg::ref_ptr<vsg::Perspective>& get_perspective() const;
    const vsg::ref_ptr<vsg::Orthographic>& get_orthographic() const;
    const vsg::ref_ptr<vsg::LookAt>& get_look_at() const;

private:
    const camera_settings_t& camera_settings;
    const vsg::ref_ptr<Mouse>& mouse;
    const vsg::ref_ptr<Keyboard>& keyboard;

    double yaw_degrees = 0.0;
    double pitch_degrees = 0.0;

    vsg::dvec3 front;
    vsg::dvec3 right;

    /// Perspective projection matrix.
    vsg::ref_ptr<vsg::Perspective> perspective;

    /// Orthographic projection matrix.
    vsg::ref_ptr<vsg::Orthographic> orthographic;

    /// LookAt view matrix.
    vsg::ref_ptr<vsg::LookAt> look_at;

private:
    /**
     * @brief Create a Perspective object.
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     *
     * @param[in] window_extent Window extent from the VSG window object.
     */
    void create_perspective(VkExtent2D window_extent);

    /**
     * @brief Create an Orthographic object.
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     */
    void create_orthographic();

    /**
     * @brief Create a LookAt object.
     *
     * Set look_at's eye height based on camera settings's initial height.
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     */
    void create_look_at();

    /**
     * @brief Create a ViewportState object based on specified window extent.
     *
     * If an error occured during initialization,
     * exit the program with std::exit (check the log file
     * (default - "logs/editor.log") for possible errors).
     *
     * @param[in] window_extent Window extent from the VSG window object.
     */
    void create_viewport_state(VkExtent2D window_extent);
};

#endif // EDITOR_CAMERA_H
