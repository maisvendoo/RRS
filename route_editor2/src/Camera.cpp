#include "editor/Camera.h"

#include "editor/settings/CameraSettings.h"

#include <Journal.h>

#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/state/ViewportState.h>

#include <vulkan/vulkan_core.h>

#include <cstdlib>

Camera::Camera(
    const camera_settings_t& camera_settings,
    VkExtent2D window_extent
)
{
    create_perspective(camera_settings, window_extent);
    create_orthographic(camera_settings);
    create_look_at(camera_settings);
    create_viewport_state(window_extent);

    projectionMatrix = perspective;
    viewMatrix = look_at;

    Journal::instance()->info("Camera is created successfully");
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

void Camera::create_perspective(
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
        std::exit(EXIT_FAILURE);
    }

    Journal::instance()->info("Perspective projection matrix is created successfully");
}

void Camera::create_orthographic(const camera_settings_t& camera_settings)
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
        std::exit(EXIT_FAILURE);
    }

    Journal::instance()->info("Orthographic projection matrix is created successfully");
}

void Camera::create_look_at(const camera_settings_t& camera_settings)
{
    look_at = vsg::LookAt::create();
    if (!look_at)
    {
        Journal::instance()->error("Failed to create LookAt view matrix");
        std::exit(EXIT_FAILURE);
    }

    // Начальный взгляд вдоль оси Y (yaw = 0, pitch = 0),
    // чтобы направление камеры было корректно с самого старта
    const double height = camera_settings.initial_height;
    look_at->eye = vsg::dvec3(0.0, 0.0, height);
    look_at->center = vsg::dvec3(0.0, 1.0, height);
    look_at->up = vsg::dvec3(0.0, 0.0, 1.0);

    Journal::instance()->info("LookAt view matrix is created successfully");
}

void Camera::create_viewport_state(VkExtent2D window_extent)
{
    viewportState = vsg::ViewportState::create(window_extent);
    if (!viewportState)
    {
        Journal::instance()->error("Failed to create viewport state");
        std::exit(EXIT_FAILURE);
    }

    Journal::instance()->info("Viewport state is created successfully");
}
