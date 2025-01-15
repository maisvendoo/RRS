#include <vsg/all.h>
#include <vsgXchange/all.h>

#include <exception>
#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[])
{
    try
    {
        vsg::CommandLine arguments(&argc, argv);

        auto options = vsg::Options::create();
        options->fileCache = vsg::getEnv("VSG_FILE_CACHE");
        options->paths = vsg::getEnvPaths("VSG_FILE_PATH");

        options->add(vsgXchange::all::create());

        arguments.read(options);

        auto windowTraits = vsg::WindowTraits::create();
        windowTraits->windowTitle = "Viewer2";
        windowTraits->width = 1600;
        windowTraits->height = 900;

        if (arguments.errors())
        {
            return arguments.writeErrorMessages(std::cerr);
        }

        auto vsgScene = vsg::read_cast<vsg::Node>("models/lz.vsgt", options);
        if (!vsgScene)
        {
            throw std::runtime_error("Failed to load VSG model");
        }

        auto viewer = vsg::Viewer::create();

        auto window = vsg::Window::create(windowTraits);
        if (!window)
        {
            throw std::runtime_error("Failed to create VSG window");
        }

        viewer->addWindow(window);

        vsg::ComputeBounds computeBounds;
        vsgScene->accept(computeBounds);
        vsg::dvec3 center = (computeBounds.bounds.min + computeBounds.bounds.max) * 0.5;
        double radius = vsg::length(computeBounds.bounds.max - computeBounds.bounds.min) * 0.6;
        double nearFarRatio = 0.0005;

        auto lookAt = vsg::LookAt::create(center + vsg::dvec3(0.0, -radius * 3.5, 0.0), center, vsg::dvec3(0.0, 0.0, 1.0));
        auto perspective = vsg::Perspective::create(30.0, static_cast<double>(window->extent2D().width) / static_cast<double>(window->extent2D().height), nearFarRatio * radius, radius * 4.5);

        auto camera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(window->extent2D()));

        viewer->addEventHandler(vsg::CloseHandler::create(viewer));
        viewer->addEventHandler(vsg::Trackball::create(camera));

        auto commandGraph = vsg::createCommandGraphForView(window, camera, vsgScene);
        viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});
        viewer->compile();

        while (viewer->advanceToNextFrame())
        {
            viewer->handleEvents();
            viewer->update();
            viewer->recordAndSubmit();
            viewer->present();
        }
    }
    catch (const vsg::Exception& exception)
    {
        for (int i = 0; i < argc; ++i)
        {
            std::cerr << argv[i] << ' ';
        }

        std::cerr << "\n[Exception] - " << exception.message << " result = " << exception.result << std::endl;

        return 1;
    }
    catch (const std::exception& exception)
    {
        for (int i = 0; i < argc; ++i)
        {
            std::cerr << argv[i] << ' ';
        }

        std::cerr << "\n[Exception] - " << exception.what() << std::endl;

        return 1;
    }
    catch (...)
    {
        for (int i = 0; i < argc; ++i)
        {
            std::cerr << argv[i] << ' ';
        }

        std::cerr << "\n[Exception] - Unknown exception" << std::endl;

        return 1;
    }

    return 0;
}
