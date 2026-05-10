#ifndef     COMPRESSOR_H
#define     COMPRESSOR_H

#include    <command-line.h>
#include    <filesystem>
#include    <vulkan/vulkan.h>
#include    <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

enum class PBRTextureRole
{
    BASE_COLOR,
    EMISSIVE,
    METALLIC_ROUGHNESS,
    NORMAL,
    OCCLUSION,
    UNKNOWN
};

struct TextureSettings
{
    PBRTextureRole role = PBRTextureRole::UNKNOWN;
    VkFormat vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    uint32_t qualityLevel = 128;
    bool isNormalMap = false;
    bool isSRGB = false;
};

int compress_model_textures(const command_line_t &cmd_line);

#endif
