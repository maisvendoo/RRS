#include <exception>
#include <iostream>
#include <ostream>
#include <vsg/all.h>
#include <vsg/app/WindowTraits.h>
#include <vsg/core/Exception.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/Options.h>
#include <vsg/utils/CommandLine.h>
#include <vsgXchange/all.h>

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
        windowTraits->debugLayer = arguments.read({"--debug", "-d"});
        windowTraits->apiDumpLayer = arguments.read({"--api", "-a"});
        windowTraits->synchronizationLayer = arguments.read("--sync");
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
