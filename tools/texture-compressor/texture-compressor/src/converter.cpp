#include    <converter.h>
#include    <fstream>
#include    <iostream>
#include    <ktx.h>
#include    <stb_image.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int Converter::run(const command_line_t &cmd_line)
{
    // Открываем модель по указанному пути
    if (!cmd_line.model_path.is_present)
    {
        std::cerr << "ERROR: missing path to GLTF-file" << std::endl;
        return -1;
    }

    fs::path gltf_path = cmd_line.model_path.value;

    // Вытаемся открыть поток ввода
    std::ifstream ifs(gltf_path);

    if (!ifs.is_open())
    {
        std::cerr << "ERROR: can't open GLTF-file" << std::endl;
        return -1;
    }

    // Пытаемся распарсить модель
    json gltf;

    try
    {
        gltf = json::parse(ifs);
    }
    catch (const std::exception &e)
    {
        std::cerr << "ERROR: JSON parse error: " << e.what() << std::endl;
        return -1;
    }

    fs::path base_dir = gltf_path.parent_path();

    int processed = 0;
    int failed = 0;

    auto img_settings = parse_gltf_textures(gltf);

    if (gltf.contains("images") && gltf["images"].is_array())
    {
        for (int img_idx = 0; img_idx < gltf["images"].size(); ++img_idx)
        {
            auto &img = gltf["images"][img_idx];

            if (!img.contains("uri"))
            {
                continue;
            }

            std::string uri = img["uri"];
            fs::path src_path = fs::weakly_canonical(base_dir / uri);

            if (!fs::exists(src_path))
            {
                std::cerr << "ERROR: Texture not found: " << src_path << std::endl;
                continue;
            }


            fs::path ktx_path = src_path;
            ktx_path.replace_extension(".ktx2");
            std::cout << "INFO: Converting: " << uri << " -> "
                      << fs::relative(ktx_path, base_dir).string()
                      << std::endl;

            auto it = img_settings.find(img_idx);
            const TextureSettings& cfg = (it != img_settings.end()) ? it->second : TextureSettings{};
            std::cout << " (Role: " << (cfg.role == PBRTextureRole::UNKNOWN ? "UNKNOWN" : "PBR")
                      << ", Format: " << (cfg.isSRGB ? "SRGB" : "UNORM") << ")\n";

            if (compress_to_ktx2(src_path, ktx_path, cfg))
            {
                img["uri"] = fs::relative(ktx_path, base_dir).string();
                img.erase("mimeType");

                if (img.contains("bufferView"))
                {
                    img.erase("bufferView");
                }

                processed++;
            }
            else
            {
                failed++;
            }
        }
    }

    fs::path out_gltf = gltf_path;
    out_gltf.replace_extension("_ktx.gltf");
    std::ofstream ofs(out_gltf);
    if (!ofs.is_open()) {
        std::cerr << "[ERR] Cannot write output glTF\n";
        return 1;
    }
    ofs << gltf.dump(2);
    ofs.close();

    return failed > 0 ? -1 : 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Converter::compress_to_ktx2(const fs::path& src_img,
                                 const fs::path& out_ktx,
                                 const TextureSettings &cfg,
                                 bool generate_mipmaps)
{

    // === 1. Загрузка изображения ===
    int w = 0, h = 0, ch = 0;
    stbi_uc* data = stbi_load(src_img.string().c_str(), &w, &h, &ch, 4);
    if (!data) {
        std::cerr << "[ERR] stbi_load: " << stbi_failure_reason() << " | " << src_img << "\n";
        return false;
    }

    const size_t size = w * h * ch;

    ktxTextureCreateInfo ci = {};
    // --format R8G8B8A8_SRGB
    ci.vkFormat        = cfg.vkFormat;

    ci.baseWidth       = static_cast<ktx_uint32_t>(w);
    ci.baseHeight      = static_cast<ktx_uint32_t>(h);
    ci.baseDepth       = 1;
    ci.numDimensions   = 2;
    ci.numLayers       = 1;
    ci.numFaces        = 1;
    ci.isArray         = KTX_FALSE;

    // В CLI мипмапы НЕ генерируются без явного --generate-mipmap
    ci.numLevels       = 1;
    ci.generateMipmaps = generate_mipmaps ? KTX_TRUE : KTX_FALSE;

    ktxTexture2* texture = nullptr;
    KTX_error_code result = ktxTexture2_Create(&ci,
                                               KTX_TEXTURE_CREATE_ALLOC_STORAGE,
                                               &texture);

    if (result != KTX_SUCCESS)
    {
        std::cerr << "Failed to create ktxTexture2: " << result << std::endl;
        return false;
    }

    result = ktxTexture_SetImageFromMemory(ktxTexture(texture),
                                           0, 0, 0,
                                           data,
                                           size);

    if (result != KTX_SUCCESS)
    {
        std::cerr << "Failed to set base image: " << result << std::endl;
        return false;
    }

    ktxBasisParams params = {};
    params.structSize = sizeof(ktxBasisParams);  // Всегда первым полем

    // --encode basis-lz
    params.codec = KTX_BASIS_CODEC_ETC1S;

    // --qlevel (по умолчанию 128)
    params.qualityLevel = cfg.qualityLevel;

    // --clevel (CLI по умолчанию 1, хотя в libktx константа = 2)
    params.etc1sCompressionLevel = 1;

    // Потоки (по умолчанию 0 = аппаратное определение всех ядер)
    params.threadCount = 0;

    // RDO-пороги (по умолчанию 1.25, применяются при qlevel <= 128)
    params.endpointRDOThreshold = 1.25f;
    params.selectorRDOThreshold = 1.25f;

    // Автоматический выбор кластеров (по умолчанию)
    params.maxEndpoints = 0;
    params.maxSelectors = 0;

    // Флаги по умолчанию
    params.verbose           = KTX_FALSE;
    params.noSSE             = KTX_FALSE;
    params.normalMap         = cfg.isNormalMap ? KTX_TRUE : KTX_FALSE;
    params.preSwizzle        = KTX_FALSE;
    params.noEndpointRDO     = KTX_FALSE;
    params.noSelectorRDO     = KTX_FALSE;

    result = ktxTexture2_CompressBasisEx(texture, &params);

    if (result != KTX_SUCCESS)
    {
        std::cerr << "Failed to compress texture: " << result << std::endl;
        return false;
    }

    result = ktxTexture_WriteToNamedFile(ktxTexture(texture), out_ktx.string().c_str());

    if (result != KTX_SUCCESS)
    {
        std::cerr << "Failed to write file: " << result << std::endl;
        return false;
    }

    ktxTexture_Destroy(ktxTexture(texture));

    stbi_image_free(data);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::map<int, Converter::TextureSettings> Converter::parse_gltf_textures(const json &gltf)
{
    std::map<int, TextureSettings> image_settings;

    if (!gltf.contains("textures") || !gltf["textures"].is_array() ||
        !gltf.contains("materials") || !gltf["materials"].is_array())
    {
        return image_settings;
    }

    auto set_role = [&](int tex_idx, PBRTextureRole role, uint32_t quality, bool is_srgb, bool is_normal) {
        if (tex_idx < 0 || tex_idx >= gltf["textures"].size()) return;
        int img_idx = gltf["textures"][tex_idx].value("source", -1);
        if (img_idx < 0 || img_idx >= gltf["images"].size()) return;

        auto& settings = image_settings[img_idx];
        // Приоритет: Normal > BaseColor > остальные. Не перезаписываем, если роль уже определена.
        if (settings.role == PBRTextureRole::UNKNOWN || role == PBRTextureRole::NORMAL) {
            settings.role = role;
            settings.qualityLevel = quality;
            settings.isNormalMap = is_normal;
            settings.isSRGB = is_srgb;
            settings.vkFormat = is_srgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
            if (role == PBRTextureRole::OCCLUSION) settings.vkFormat = VK_FORMAT_R8_UNORM;
        }
    };

    for (const auto& mat : gltf["materials"])
    {
        if (mat.contains("pbrMetallicRoughness"))
        {
            auto& pbr = mat["pbrMetallicRoughness"];
            if (pbr.contains("baseColorTexture"))
                set_role(pbr["baseColorTexture"]["index"], PBRTextureRole::BASE_COLOR, 160, true, false);

            if (pbr.contains("metallicRoughnessTexture"))
                set_role(pbr["metallicRoughnessTexture"]["index"], PBRTextureRole::METALLIC_ROUGHNESS, 128, false, false);
        }

        if (mat.contains("normalTexture"))
            set_role(mat["normalTexture"]["index"], PBRTextureRole::NORMAL, 192, false, true);

        if (mat.contains("emissiveTexture"))
            set_role(mat["emissiveTexture"]["index"], PBRTextureRole::EMISSIVE, 128, true, false);
        if (mat.contains("occlusionTexture"))
            set_role(mat["occlusionTexture"]["index"], PBRTextureRole::OCCLUSION, 96, false, false);
    }
    return image_settings;
}
