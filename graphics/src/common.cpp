#include "graphics/common.h"

#include <vsg/io/FileSystem.h>
#include <vsg/io/Options.h>
#include <vsg/utils/SharedObjects.h>
#include <vsgXchange/all.h>

vsg::ref_ptr<vsg::Options> create_default_vsg_options()
{
    const auto options{vsg::Options::create()};
    options->sharedObjects = vsg::SharedObjects::create();
    options->fileCache = vsg::getEnv("VSG_FILE_CACHE");
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    options->add(vsgXchange::all::create());

    return options;
}

VkSampleCountFlags get_vk_sample_count_flag(int samples)
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
