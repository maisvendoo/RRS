#ifndef     CONVERTER_H
#define     CONVERTER_H

#include    <command-line.h>
#include    <filesystem>
#include    <vulkan/vulkan.h>
#include    <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Converter
{
public:

    Converter() = default;

    ~Converter() = default;

    int run(const command_line_t &cmd_line);

private:

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

    bool compress_to_ktx2(const fs::path& src_img, const fs::path& out_ktx, const TextureSettings& cfg, bool generate_mipmaps = false);

    std::map<int, TextureSettings> parse_gltf_textures(const json& gltf);
};

#endif
