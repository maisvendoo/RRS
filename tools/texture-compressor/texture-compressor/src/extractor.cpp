#include    <extractor.h>
#include    <iostream>
#include    <fstream>
#include    <iostream>
#include    <ktx.h>
#include    <filesystem>
#include    <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

#define     STB_IMAGE_WRITE_IMPLEMENTATION
#include    <stb_image_write.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool extract_to_png(const fs::path &src_ktx, const fs::path &out_png)
{
    ktxTexture2 *texture = NULL;
    KTX_error_code result;

    result = ktxTexture2_CreateFromNamedFile(src_ktx.string().c_str(),
                                             KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
                                             &texture);

    if (result != KTX_SUCCESS)
    {
        std::cerr << "[ERR] error of texture loading" << std::endl;
        return false;
    }

    if (ktxTexture2_NeedsTranscoding(texture))
    {
        result = ktxTexture2_TranscodeBasis(texture, KTX_TTF_RGBA32, 0);

        if (result != KTX_SUCCESS)
        {
            std::cerr << "[ERR] error of texture transcoding" << std::endl;
            ktxTexture2_Destroy(texture);

            return false;
        }
    }

    ktx_size_t offset = 0;

    result = ktxTexture2_GetImageOffset(texture, 0, 0, 0, &offset);

    if (result != KTX_SUCCESS)
    {
        std::cerr << "[ERR] error of get image offset" << std::endl;
        return false;
    }

    uint8_t *pixel_data = texture->pData + offset;
    uint32_t width = texture->baseWidth;
    uint32_t height = texture->baseHeight;

    if (!pixel_data || width == 0 || height == 0)
    {
        std::cerr << "[ERR] missign valid image data in texture" << std::endl;
        ktxTexture2_Destroy(texture);

        return false;
    }

    if (!stbi_write_png(out_png.string().c_str(), width, height, 4, pixel_data, width * 4))
    {
        std::cerr << "[ERR] wirte PNG file error" << std::endl;
        ktxTexture2_Destroy(texture);

        return false;
    }

    std::cout << "[INF] file " << out_png.string() << " saved success" << std::endl;
    ktxTexture2_Destroy(texture);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int extract_models_textures(const command_line_t &cmd_line)
{
    // Открываем модель по указанному пути
    if (!cmd_line.model_path.has_value())
    {
        std::cerr << "[ERR] missing path to GLTF-file" << std::endl;
        return -1;
    }

    fs::path gltf_path = cmd_line.model_path.value();

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

            fs::path png_path = src_path;
            png_path.replace_extension(".png");
            std::cout << "[INF] Converting: " << uri << " -> "
                      << fs::relative(png_path, base_dir).string()
                      << std::endl;

            if (extract_to_png(src_path, png_path))
            {
                img["uri"] = fs::relative(png_path, base_dir).string();

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
        out_gltf = gltf_path.parent_path() / (gltf_path.stem().string() + "_png.gltf");

    std::ofstream ofs(out_gltf);

    if (!ofs.is_open())
    {
        std::cerr << "[ERR] Cannot write output glTF\n";
        return -1;
    }

    ofs << gltf.dump(2);
    ofs.close();

    return failed > 0 ? -1 : 0;

    return 0;
}
