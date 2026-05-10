#include    <extractor.h>
#include    <iostream>
#include    <fstream>
#include    <iostream>
#include    <ktx.h>
#include    <filesystem>
#include    <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool extract_to_png(const fs::path &src_ktx, const fs::path &out_png)
{
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int extract_models_textures(const command_line_t &cmd_line)
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
