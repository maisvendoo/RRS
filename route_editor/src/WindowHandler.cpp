#include "WindowHandler.h"

#include "Camera.h"
#include "EditorContext.h"
#include "Journal.h"
#include "settings/WindowSettings.h"

#include <vsg/app/Camera.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/Window.h>
#include <vsg/app/WindowTraits.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/ui/WindowEvent.h>

#include <vulkan/vulkan_core.h>

static VkSampleCountFlags samples_bit_flag(int samples);

WindowHandler::WindowHandler(
        const window_settings_t& window_settings,
        vsg::ref_ptr<vsg::Window>& window,
        const vsg::ref_ptr<Camera>& camera
    )
    : camera(camera)
{
    const auto window_traits = vsg::WindowTraits::create();
    window_traits->x = window_settings.pos_x;
    window_traits->y = window_settings.pos_y;
    window_traits->width = window_settings.width;
    window_traits->height = window_settings.height;
    window_traits->fullscreen = window_settings.fullscreen;
    window_traits->screenNum = window_settings.screen_number;
    window_traits->windowTitle = window_settings.title;

    window_traits->swapchainPreferences.presentMode =
        window_settings.vsync ? VK_PRESENT_MODE_FIFO_KHR
                              : VK_PRESENT_MODE_MAILBOX_KHR;

    window_traits->samples = samples_bit_flag(window_settings.samples);

    window = vsg::Window::create(window_traits);
    if (!window)
    {
        Journal::instance()->error("Failed to create window");
    }
}

void WindowHandler::apply(vsg::ConfigureWindowEvent& configureWindow)
{
    const std::uint32_t width{configureWindow.width};
    const std::uint32_t height{configureWindow.height};

    if (camera)
    {
        camera->get_perspective()->aspectRatio =
            static_cast<double>(width) / static_cast<double>(height);

        camera->get_camera()->viewportState->set(0, 0, width, height);
    }
}

VkSampleCountFlags samples_bit_flag(int samples)
{
    if (samples >= 8)
    {
        return VK_SAMPLE_COUNT_8_BIT;
    }
    else if (samples >= 4)
    {
        return VK_SAMPLE_COUNT_4_BIT;
    }
    else if (samples >= 2)
    {
        return VK_SAMPLE_COUNT_2_BIT;
    }
    else
    {
        return VK_SAMPLE_COUNT_1_BIT;
    }
}
