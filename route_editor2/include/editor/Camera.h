#ifndef EDITOR_CAMERA_H
#define EDITOR_CAMERA_H

#include <vsg/app/Camera.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>

#include <vulkan/vulkan_core.h>

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
     * Create perspective and orthographic projection matrices, as well
     * as a LookAt view matrix, and ViewportState based on the
     * camera_settings and window_extent parameters. Then
     * set the projection matrix to perspective mode.
     *
     * @param[in] camera_settings The camera settings object.
     * @param[in] window_extent Window extent from the VSG window object.
     * @param[out] success true if initialization was successful,
     *                     false - otherwise.
     */
    Camera(
        const camera_settings_t& camera_settings,
        VkExtent2D window_extent,
        bool& success
    );

    /**
     * @brief Destroy the Camera object.
     */
    ~Camera();

    /**
     * @brief Get the perspective object.
     *
     * @return The perspective object.
     */
    const vsg::ref_ptr<vsg::Perspective>& get_perspective() const;

    /**
     * @brief Get the orthographic object.
     *
     * @return The orthographic object.
     */
    const vsg::ref_ptr<vsg::Orthographic>& get_orthographic() const;

    /**
     * @brief Get the look_at object
     *
     * @return The look_at object.
     */
    const vsg::ref_ptr<vsg::LookAt>& get_look_at() const;

private:
    /// Perspective projection matrix.
    vsg::ref_ptr<vsg::Perspective> perspective;

    /// Orthographic projection matrix.
    vsg::ref_ptr<vsg::Orthographic> orthographic;

    /// LookAt view matrix.
    vsg::ref_ptr<vsg::LookAt> look_at;

private:
    /**
     * @brief Create a perspective object.
     *
     * @param[in] camera_settings The camera settings object.
     * @param[in] window_extent Window extent from the VSG window object.
     * @return true if perspective was created successfully.
     * @return false - otherwise.
     */
    bool create_perspective(
        const camera_settings_t& camera_settings,
        VkExtent2D window_extent
    );

    /**
     * @brief Create an orthographic object.
     *
     * @param camera_settings The camera settings object.
     * @return true if orthographic was created successfully.
     * @return false - otherwise.
     */
    bool create_orthographic(const camera_settings_t& camera_settings);

    /**
     * @brief Create a LookAt object.
     *
     * @param camera_settings The camera settings object.
     * @return true if LookAt was created successfully.
     * @return false - otherwise.
     */
    bool create_look_at(const camera_settings_t& camera_settings);

    /**
     * @brief Create a ViewportState object.
     *
     * @param window_extent Window extent from the VSG window object.
     * @return true if ViewportState was created successfully.
     * @return false - otherwise.
     */
    bool create_viewport_state(VkExtent2D window_extent);
};

#endif // EDITOR_CAMERA_H
