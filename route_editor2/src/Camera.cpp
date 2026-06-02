#include "editor/Camera.h"

#include "editor/check_macro.h"
#include "editor/settings/CameraSettings.h"

#include <Journal.h>

#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/state/ViewportState.h>

#include <vulkan/vulkan_core.h>

Camera::Camera(
    const camera_settings_t& camera_settings,
    VkExtent2D window_extent,
    bool& success
)
{
    CHECK(create_perspective(camera_settings, window_extent), success)
    CHECK(create_orthographic(camera_settings), success)
    CHECK(create_look_at(camera_settings), success)
    CHECK(create_viewport_state(window_extent), success)

    projectionMatrix = perspective;
    viewMatrix = look_at;

    Journal::instance()->info("Camera is created successfully");

    success = true;
}

Camera::~Camera() = default;

const vsg::ref_ptr<vsg::Perspective>& Camera::get_perspective() const
{
    return perspective;
}

const vsg::ref_ptr<vsg::Orthographic>& Camera::get_orthographic() const
{
    return orthographic;
}

const vsg::ref_ptr<vsg::LookAt>& Camera::get_look_at() const
{
    return look_at;
}

bool Camera::create_perspective(
    const camera_settings_t& camera_settings,
    VkExtent2D window_extent
)
{
    perspective = vsg::Perspective::create(
        camera_settings.fovy_degrees,
        static_cast<double>(window_extent.width) /
            static_cast<double>(window_extent.height),
        camera_settings.zNear,
        camera_settings.view_distance
    );

    if (!perspective)
    {
        Journal::instance()->error("Failed to create perspective projection matrix");
        return false;
    }

    Journal::instance()->info("Perspective projection matrix is created successfully");
    return true;
}

bool Camera::create_orthographic(const camera_settings_t& camera_settings)
{
    orthographic = vsg::Orthographic::create(
        -1.0,
        1.0,
        -1.0,
        1.0,
        camera_settings.zNear,
        camera_settings.view_distance
    );

    if (!orthographic)
    {
        Journal::instance()->error("Failed to create orthographic projection matrix");
        return false;
    }

    Journal::instance()->info("Orthographic projection matrix is created successfully");
    return true;
}

bool Camera::create_look_at(const camera_settings_t& camera_settings)
{
    look_at = vsg::LookAt::create();
    if (!look_at)
    {
        Journal::instance()->error("Failed to create LookAt view matrix");
        return false;
    }

    look_at->eye.z = look_at->center.z = camera_settings.initial_height;

    Journal::instance()->info("LookAt view matrix is created successfully");
    return true;
}

bool Camera::create_viewport_state(VkExtent2D window_extent)
{
    viewportState = vsg::ViewportState::create(window_extent);
    if (!viewportState)
    {
        Journal::instance()->error("Failed to create viewport state");
        return false;
    }

    Journal::instance()->info("Viewport state is created successfully");
    return true;
}
