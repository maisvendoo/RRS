#include "editor/Camera.h"

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
    const double window_width = static_cast<double>(window_extent.width);
    const double window_height = static_cast<double>(window_extent.height);

    Journal* const journal = Journal::instance();

    // TODO: Check perspective allocation?
    perspective = vsg::Perspective::create(
        camera_settings.fovy_degrees,
        window_width / window_height,
        camera_settings.zNear,
        camera_settings.view_distance
    );

    journal->info("Perspective projection matrix is created successfully");

    // TODO: Check orthographic allocation?
    orthographic = vsg::Orthographic::create(
        -1.0,
        1.0,
        -1.0,
        1.0,
        camera_settings.zNear,
        camera_settings.view_distance
    );

    journal->info("Orthographic projection matrix is created successfully");

    // TODO: Check look_at allocation?
    look_at = vsg::LookAt::create();

    look_at->eye.z = look_at->center.z = camera_settings.initial_height;

    journal->info("LookAt view matrix is created successfully");

    projectionMatrix = perspective;
    viewMatrix = look_at;
    // TODO: Check viewportState allocation?
    viewportState = vsg::ViewportState::create(window_extent);

    journal->info("Camera is created successfully");

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
