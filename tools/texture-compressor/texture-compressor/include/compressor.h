#ifndef     COMPRESSOR_H
#define     COMPRESSOR_H

#include    <command-line.h>
#include    <vulkan/vulkan.h>

enum class PBRTextureRole
{
    BASE_COLOR,
    EMISSIVE,
    METALLIC_ROUGHNESS,
    NORMAL,
    OCCLUSION,
    ORM,        ///< Совмещённая карта: R - occlusion, G - roughness, B - metallic
    UNKNOWN
};

struct TextureSettings
{
    PBRTextureRole role = PBRTextureRole::UNKNOWN;
    VkFormat vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    uint32_t qualityLevel = 128;
    bool isNormalMap = false;
    bool isSRGB = false;
    bool isORM = false;
};

int compress_model_textures(const command_line_t &cmd_line);

#endif
