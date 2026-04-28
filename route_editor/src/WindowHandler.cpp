#include "WindowHandler.h"

#include "EditorContext.h"
#include "Journal.h"
#include "Settings.h"

#include <vsg/app/Camera.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/Window.h>
#include <vsg/app/WindowTraits.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/ui/WindowEvent.h>

#include <vulkan/vulkan_core.h>

static VkSampleCountFlags samples_bit_flag(int samples)
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

WindowHandler::WindowHandler(EditorContext& context)
    : context_(context)
{
    const settings_t& settings = context.settings;

    const auto window_traits = vsg::WindowTraits::create();
    window_traits->x = settings.window_x;
    window_traits->y = settings.window_y;
    window_traits->width = settings.window_width;
    window_traits->height = settings.window_height;
    window_traits->fullscreen = settings.fullscreen;
    window_traits->screenNum = settings.screen_number;
    window_traits->windowTitle = settings.window_title;

    window_traits->swapchainPreferences.presentMode =
        settings.vsync ? VK_PRESENT_MODE_FIFO_KHR
                       : VK_PRESENT_MODE_MAILBOX_KHR;

    window_traits->samples = samples_bit_flag(settings.samples);

    context.window = vsg::Window::create(window_traits);
    if (!context.window)
    {
        Journal::instance()->error("Failed to create window");
    }
}

void WindowHandler::apply(vsg::ConfigureWindowEvent& configureWindow)
{
    context_.perspective->aspectRatio =
        static_cast<double>(configureWindow.width) /
        static_cast<double>(configureWindow.height);

    context_.camera->viewportState->set(0, 0,
        configureWindow.width, configureWindow.height);
}
