#include    <compressor.h>

#include    <fstream>
#include    <iostream>
#include    <ktx.h>
#include    <thread>

#define     STB_IMAGE_IMPLEMENTATION
#include    <stb_image.h>
#define     STB_IMAGE_RESIZE_IMPLEMENTATION
#include    <stb_image_resize.h>

#include    <split-by-comma.h>

#include    <filesystem>
#include    <set>
#include    <vulkan/vulkan.h>
#include    <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool compress_to_ktx2(const fs::path& src_img,
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

    // Загружаем базовый уровень (0). Данные всегда RGBA (w*h*4), формат
    // ci.vkFormat выбран RGBA-совместимым (см. parse_gltf_textures),
    // поэтому объём данных совпадает с форматом.
    auto* baseTex = ktxTexture(texture);
    result = ktxTexture_SetImageFromMemory(baseTex, 0, 0, 0, data, w * h * 4);

    if (result != KTX_SUCCESS)
    {
        std::cerr << "[ERR] Failed to set base image data: " << result << std::endl;
        ktxTexture_Destroy(ktxTexture(texture));
        stbi_image_free(data);
        return false;
    }

    // Буферы мип-уровней выделяются через new[] и освобождаются через delete[].
    // Исходные данные (data) приходят из stbi_load и освобождаются stbi_image_free.
    std::vector<stbi_uc *> mip_buffers;

    // Если нужны мипы — генерируем их вручную, последовательно уменьшая
    // размеры текстуры в два раза на каждом уровне
    if (generate_mipmaps && ci.numLevels > 1)
    {
        stbi_uc* prevLevelData = data;
        uint32_t prevW = w, prevH = h;

        for (uint32_t level = 1; level < ci.numLevels; ++level)
        {
            uint32_t curW = std::max(1u, prevW >> 1);
            uint32_t curH = std::max(1u, prevH >> 1);

            stbi_uc* resized = new stbi_uc[curW * curH * 4];

            // Ресемплинг
            stbir_resize_uint8(
                prevLevelData, prevW, prevH, 0,
                resized, curW, curH, 0, 4
                );

            // Загружаем уровень в ktxTexture
            result = ktxTexture_SetImageFromMemory(
                baseTex, level, 0, 0,
                resized, curW * curH * 4
                );

            if (result != KTX_SUCCESS)
            {
                std::cerr << "[ERR] Failed to set image data for mip level "
                          << level << ": " << result << std::endl;
                delete[] resized;
                for (auto* b : mip_buffers)
                    delete[] b;
                ktxTexture_Destroy(ktxTexture(texture));
                stbi_image_free(data);
                return false;
            }

            mip_buffers.push_back(resized);
            prevLevelData = resized;
            prevW = curW;
            prevH = curH;
        }
    }

    // Подготовка данных для сжатия
    ktxBasisParams params = {};
    params.structSize = sizeof(ktxBasisParams);

    // Карты данных (ORM, metallic/roughness, occlusion, normal) должны сжиматься
    // так, чтобы KTX2 гарантированно оставался 4-канальным (RGBA): только тогда
    // vsgXchange транскодирует его в BC7_RGBA. Для 3-канальных ETC1S он выбирает
    // BC1_RGB (цветовой кодек), который искажает карты данных (розовый оттенок).
    // UASTC LDR 4x4 всегда кодирует RGBA, поэтому используем его для этих карт.
    const bool is_data_map =
        cfg.isNormalMap ||
        cfg.isORM ||
        cfg.role == PBRTextureRole::METALLIC_ROUGHNESS ||
        cfg.role == PBRTextureRole::OCCLUSION;

    params.codec = is_data_map ? KTX_BASIS_CODEC_UASTC_LDR_4x4
                               : KTX_BASIS_CODEC_ETC1S;

    // Параметры ETC1S (только для цветовых карт)
    params.etc1sCompressionLevel = 2; // KTX_ETC1S_DEFAULT_COMPRESSION_LEVEL = 2

    // --qlevel (по умолчанию 128)
    params.qualityLevel = cfg.qualityLevel;

    // Параметры UASTC (для карт данных)
    if (is_data_map)
    {
        // Уровень качества UASTC: DEFAULT (2) - хороший баланс скорость/качество
        params.uastcFlags = KTX_PACK_UASTC_LEVEL_DEFAULT;
        params.uastcRDO = KTX_FALSE;
    }

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

        // Карты с данными roughness/metallic/AO чувствительны к RDO-оптимизации —
        // отключаем её, чтобы не искажать каналы.
        const bool is_data_map =
            cfg.isORM ||
            cfg.role == PBRTextureRole::METALLIC_ROUGHNESS ||
            cfg.role == PBRTextureRole::OCCLUSION;

        if (is_data_map)
        {
            params.noEndpointRDO = KTX_TRUE;
            params.noSelectorRDO = KTX_TRUE;
        }
        else
        {
            params.endpointRDOThreshold = 1.25f;
            params.selectorRDOThreshold = 1.25f;
            params.noEndpointRDO = KTX_FALSE;
            params.noSelectorRDO = KTX_FALSE;
        }
    }

    params.verbose = KTX_FALSE;
    params.noSSE = KTX_FALSE;

    result = ktxTexture2_CompressBasisEx(texture, &params);

    if (result != KTX_SUCCESS)
    {
        std::cerr << "[ERR] Failed to compress texture: " << result << std::endl;
        ktxTexture_Destroy(ktxTexture(texture));
        for (auto* b : mip_buffers)
            delete[] b;
        stbi_image_free(data);
        return false;
    }

    result = ktxTexture_WriteToNamedFile(ktxTexture(texture), out_ktx.string().c_str());

    if (result != KTX_SUCCESS)
    {
        std::cerr << "[ERR] Failed to write file: " << result << std::endl;
        ktxTexture_Destroy(ktxTexture(texture));
        for (auto* b : mip_buffers)
            delete[] b;
        stbi_image_free(data);
        return false;
    }

    // Уничтожаем текстуру в памяти
    ktxTexture_Destroy(ktxTexture(texture));

    // Освобождаем буферы мип-уровней (new[] → delete[])
    for (auto* b : mip_buffers)
    {
        delete[] b;
    }

    // Освобождаем исходные данные, загруженные stbi_load
    stbi_image_free(data);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::map<int, TextureSettings> parse_gltf_textures(const json &gltf)
{
    std::map<int, TextureSettings> image_settings;

    if (!gltf.contains("textures") || !gltf["textures"].is_array() ||
        !gltf.contains("materials") || !gltf["materials"].is_array() ||
        !gltf.contains("images") || !gltf["images"].is_array())
    {
        return image_settings;
    }

    // Собираем множество image-индексов по каждой роли из структуры материалов.
    // Это позволяет однозначно определить совмещённую ORM-карту: один и тот же
    // image подключён и как metallicRoughnessTexture, и как occlusionTexture.
    std::set<int> used_mr;
    std::set<int> used_occ;
    std::set<int> used_base;
    std::set<int> used_emis;
    std::set<int> used_norm;

    auto src_of = [&](int tex_idx) -> int {
        if (tex_idx < 0 || tex_idx >= gltf["textures"].size()) return -1;
        return gltf["textures"][tex_idx].value("source", -1);
    };

    for (const auto& mat : gltf["materials"])
    {
        if (mat.contains("pbrMetallicRoughness"))
        {
            auto& pbr = mat["pbrMetallicRoughness"];
            if (pbr.contains("baseColorTexture"))
                used_base.insert(src_of(pbr["baseColorTexture"]["index"]));
            if (pbr.contains("metallicRoughnessTexture"))
                used_mr.insert(src_of(pbr["metallicRoughnessTexture"]["index"]));
        }

        if (mat.contains("normalTexture"))
            used_norm.insert(src_of(mat["normalTexture"]["index"]));

        if (mat.contains("emissiveTexture"))
            used_emis.insert(src_of(mat["emissiveTexture"]["index"]));

        if (mat.contains("occlusionTexture"))
            used_occ.insert(src_of(mat["occlusionTexture"]["index"]));
    }

    const int images_count = gltf["images"].size();

    for (int img = 0; img < images_count; ++img)
    {
        const bool is_mr   = used_mr.count(img)   > 0;
        const bool is_occ  = used_occ.count(img)  > 0;
        const bool is_base = used_base.count(img) > 0;
        const bool is_emis = used_emis.count(img) > 0;
        const bool is_norm = used_norm.count(img) > 0;

        auto& s = image_settings[img];

        // Нормаль — самостоятельная роль, перекрывает остальные.
        if (is_norm)
        {
            s.role = PBRTextureRole::NORMAL;
            s.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
            s.qualityLevel = 192;
            s.isNormalMap = true;
            continue;
        }

        // Совмещённая ORM-карта: R - occlusion, G - roughness, B - metallic.
        if (is_mr && is_occ)
        {
            s.role = PBRTextureRole::ORM;
            s.isORM = true;
            s.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
            s.qualityLevel = 128;
            continue;
        }

        if (is_mr)
        {
            s.role = PBRTextureRole::METALLIC_ROUGHNESS;
            s.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
            s.qualityLevel = 128;
            continue;
        }

        // Отдельная AO-карта: не форсируем R8, чтобы объём данных RGBA
        // (w*h*4) совпадал с форматом.
        if (is_occ)
        {
            s.role = PBRTextureRole::OCCLUSION;
            s.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
            s.qualityLevel = 96;
            continue;
        }

        if (is_base)
        {
            s.role = PBRTextureRole::BASE_COLOR;
            s.vkFormat = VK_FORMAT_R8G8B8A8_SRGB;
            s.qualityLevel = 160;
            s.isSRGB = true;
            continue;
        }

        if (is_emis)
        {
            s.role = PBRTextureRole::EMISSIVE;
            s.vkFormat = VK_FORMAT_R8G8B8A8_SRGB;
            s.qualityLevel = 128;
            s.isSRGB = true;
            continue;
        }

        s.role = PBRTextureRole::UNKNOWN;
    }

    return image_settings;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int compress_model_textures(const command_line_t &cmd_line)
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
