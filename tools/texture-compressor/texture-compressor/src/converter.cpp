#include    <converter.h>
#include    <fstream>
#include    <iostream>
#include    <ktx.h>
#include    <thread>

#define     STB_IMAGE_IMPLEMENTATION
#include    <stb_image.h>
#define     STB_IMAGE_RESIZE_IMPLEMENTATION
#include    <stb_image_resize.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<std::string_view> split_by_comma_view(std::string_view str)
{
    std::vector<std::string_view> tokens;
    size_t start = 0;
    size_t end = str.find(',');

    while (end != std::string_view::npos)
    {
        tokens.emplace_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(',', start);
    }
    tokens.emplace_back(str.substr(start));
    return tokens;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int Converter::run(const command_line_t &cmd_line)
{
    // Открываем модель по указанному пути
    if (!cmd_line.model_path.is_present)
    {
        std::cerr << "[ERR] missing path to GLTF-file" << std::endl;
        return -1;
    }

    fs::path gltf_path = cmd_line.model_path.value;

    // Вытаемся открыть поток ввода
    std::ifstream ifs(gltf_path);

    if (!ifs.is_open())
    {
        std::cerr << "[ERR] can't open GLTF-file" << std::endl;
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
        std::cerr << "[ERR] JSON parse error: " << e.what() << std::endl;
        return -1;
    }

    fs::path base_dir = gltf_path.parent_path();

    int processed = 0;
    int failed = 0;

    auto img_settings = parse_gltf_textures(gltf);

    // Формируем список пропускаемых текструр
    auto skiped_textures = split_by_comma_view(cmd_line.skip_textures.value);

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
                std::cerr << "[ERR] Texture not found: " << src_path << std::endl;
                continue;
            }

            // Проверяем, не надо ли пропустить эту текстуру
            bool is_skip_texture = false;

            for (auto skip_tex : skiped_textures)
            {
                if (skip_tex == src_path.filename().string())
                {
                    is_skip_texture = true;
                    break;
                }
            }

            // Пропускаем текстуру из списка
            if (is_skip_texture)
            {
                std::cout << "[INF] Skip texture " << src_path << std::endl;
                continue;
            }

            fs::path ktx_path = src_path;
            ktx_path.replace_extension(".ktx2");
            std::cout << "[INF] Converting: " << uri << " -> "
                      << fs::relative(ktx_path, base_dir).string()
                      << std::endl;

            auto it = img_settings.find(img_idx);
            const TextureSettings& cfg = (it != img_settings.end()) ? it->second : TextureSettings{};
            std::cout << " (Role: " << (cfg.role == PBRTextureRole::UNKNOWN ? "UNKNOWN" : "PBR")
                      << ", Format: " << (cfg.isSRGB ? "SRGB" : "UNORM") << ")\n";

            if (compress_to_ktx2(src_path, ktx_path, cfg, cmd_line.generate_mipmaps, cmd_line.ignore_existed))
            {
                img["uri"] = fs::relative(ktx_path, base_dir).string();
                img.erase("mimeType");

                if (img.contains("bufferView"))
                {
                    img.erase("bufferView");
                }

                // Удаляем исходную текстуру, если это требуется
                if (cmd_line.delete_src)
                {
                    fs::remove(src_path);
                }

                processed++;
            }
            else
            {
                failed++;
            }
        }
    }

    fs::path out_gltf;

    if (cmd_line.overwrite_gltf)
        out_gltf = gltf_path;
    else
        out_gltf = gltf_path.parent_path() / (gltf_path.stem().string() + "_ktx.gltf");

    std::ofstream ofs(out_gltf);

    if (!ofs.is_open())
    {
        std::cerr << "[ERR] Cannot write output glTF\n";
        return -1;
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
                                 bool generate_mipmaps,
                                 bool ignore)
{
    if (ignore)
    {
        if (fs::exists(out_ktx))
        {
            std::cout << "[INF] Texture " << out_ktx << " already exist. Skiped" << std::endl;
            return true;
        }
    }

    // Загрузка оригинальной текстуры
    int w = 0, h = 0, ch = 0;   

    // Всегда берем 4 канала, если нет альфы, то создается альфа равная 255
    stbi_uc* data = stbi_load(src_img.string().c_str(), &w, &h, &ch, 4);

    if (!data)
    {
        std::cerr << "[ERR] stbi_load: " << stbi_failure_reason() << " | " << src_img << std::endl;
        return false;
    }    

    // Вычисление количества мип-уровней
    auto calcMipLevels = [](uint32_t w, uint32_t h) -> uint32_t {
        return static_cast<uint32_t>(floor(log2(std::max(w, h))) + 1);
    };

    // Заполняем структуру, несущую информацию о будущей текстре
    ktxTextureCreateInfo ci = {};
    ci.vkFormat        = cfg.vkFormat; // формат входных данных
    ci.baseWidth       = static_cast<ktx_uint32_t>(w); // базовая ширина
    ci.baseHeight      = static_cast<ktx_uint32_t>(h); // базовая выоста
    ci.baseDepth       = 1; // базовая глубина
    ci.numDimensions   = 2; // базовая размерность (2D)
    ci.numLayers       = 1;
    ci.numFaces        = 1;
    ci.isArray         = KTX_FALSE;

    // Число mipmap-уровней определяем ЯВНО, так как сами их сгенерируем
    ci.numLevels       = generate_mipmaps ? calcMipLevels(w, h) : 1;
    // Не полагаемся на аппратную генерацию - это для несжатых текстур
    ci.generateMipmaps = KTX_FALSE;

    // Создаем текстуру
    ktxTexture2* texture = nullptr;
    KTX_error_code result = ktxTexture2_Create(&ci,
                                               KTX_TEXTURE_CREATE_ALLOC_STORAGE,
                                               &texture);

    if (result != KTX_SUCCESS)
    {
        std::cerr << "[ERR] Failed to create ktxTexture2: " << result << std::endl;
        return false;
    }

    // Загружаем базовый уровень (0)
    auto* baseTex = ktxTexture(texture);
    ktxTexture_SetImageFromMemory(baseTex, 0, 0, 0, data, w * h * 4);

    // Массив под указатели на данные всех уровней, не удаляем эти данные до сжатия
    // и сохранения текстуры на диск!
    std::vector<stbi_uc *> levels_data;

    // Если нужны мипы — генерируем их вручную, последовательно уменашая
    // рзмеры текструы в два раза на каждом уровне
    if (generate_mipmaps && ci.numLevels > 1)
    {
        stbi_uc* prevLevelData = data;
        uint32_t prevW = w, prevH = h;

        for (uint32_t level = 1; level < ci.numLevels; ++level)
        {
            levels_data.push_back(prevLevelData);

            uint32_t curW = std::max(1u, prevW >> 1);
            uint32_t curH = std::max(1u, prevH >> 1);

            stbi_uc* resized = new stbi_uc[curW * curH * 4];

            // Ресемплинг
            stbir_resize_uint8(
                prevLevelData, prevW, prevH, 0,
                resized, curW, curH, 0, 4
                );

            // Загружаем уровень в ktxTexture
            ktxTexture_SetImageFromMemory(
                baseTex, level, 0, 0,
                resized, curW * curH * 4
                );

            prevLevelData = resized;
            prevW = curW;
            prevH = curH;
        }
    }

    // Подготовка данных для сжатия
    ktxBasisParams params = {};
    params.structSize = sizeof(ktxBasisParams);

    // КРИТИЧНО: Обязательно устанавливаем уровень сжатия ETC1S
    // Ваше значение 1 - допустимо, но используйте константу для ясности
    params.etc1sCompressionLevel = 2; // KTX_ETC1S_DEFAULT_COMPRESSION_LEVEL = 2

    // --encode basis-lz
    params.codec = KTX_BASIS_CODEC_ETC1S;

    // --qlevel (по умолчанию 128)
    params.qualityLevel = cfg.qualityLevel;

    // Потоки - ЯВНО указываем количество наших ядер
    unsigned int numThreads = std::thread::hardware_concurrency();
    params.threadCount = (numThreads == 0) ? 1 : numThreads;

    // RDO-пороги - важны для normal maps!
    if (cfg.isNormalMap)
    {
        // Для normal map ДОЛЖНЫ быть отключены RDO оптимизации
        // Согласно документации: "Tunes codec parameters for better quality on normal maps
        // (no selector RDO, no endpoint RDO)"
        params.normalMap = KTX_TRUE;
        params.noEndpointRDO = KTX_TRUE;  // Отключаем endpoint RDO
        params.noSelectorRDO = KTX_TRUE;  // Отключаем selector RDO

        // Для normal maps не используем preSwizzle - он для метаданных KTXswizzle
        // В вашем случае нет такой метаинформации
        params.preSwizzle = KTX_FALSE;

        // Также не используем inputSwizzle, если не нужно менять порядок каналов
        params.inputSwizzle[0] = 'r'; // Если нужно переставить каналы
        params.inputSwizzle[1] = 'g';
        params.inputSwizzle[2] = 'b';
        params.inputSwizzle[3] = 'a';
    } else
    {
        params.normalMap = KTX_FALSE;
        params.endpointRDOThreshold = 1.25f;
        params.selectorRDOThreshold = 1.25f;
        params.noEndpointRDO = KTX_FALSE;
        params.noSelectorRDO = KTX_FALSE;
    }

    params.verbose = KTX_FALSE;
    params.noSSE = KTX_FALSE;

    result = ktxTexture2_CompressBasisEx(texture, &params);

    if (result != KTX_SUCCESS)
    {
        std::cerr << "[ERR] Failed to compress texture: " << result << std::endl;
        return false;
    }

    result = ktxTexture_WriteToNamedFile(ktxTexture(texture), out_ktx.string().c_str());

    if (result != KTX_SUCCESS)
    {
        std::cerr << "[ERR] Failed to write file: " << result << std::endl;
        return false;
    }

    // Уничтожаем текстуру в памяти
    ktxTexture_Destroy(ktxTexture(texture));

    // Освобождаем память всех мипмап-уровней
    for (auto d : levels_data)
    {
        stbi_image_free(d);
    }

    // Уходим довольные, как слон, проделанной работой
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
        if (settings.role == PBRTextureRole::UNKNOWN || role == PBRTextureRole::NORMAL)
        {
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
