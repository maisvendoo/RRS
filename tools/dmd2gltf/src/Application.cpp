#include    <Application.h>
#include    <Geometry.h>
#include    <filesystem-utils.h>
#include    <Logger.h>

#include    <algorithm>
#include    <chrono>
#include    <cmath>
#include    <cstdint>
#include    <filesystem>
#include    <fstream>
#include    <iostream>
#include    <map>
#include    <set>
#include    <string>
#include    <thread>
#include    <utility>
#include    <vector>

#include    <vulkan/vulkan.h>
#include    <ktx.h>

#define STB_IMAGE_IMPLEMENTATION
#include    <stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include    <stb_image_resize.h>

namespace fs = std::filesystem;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static bool is_slash(char ch)
{
    return ch == '/' || ch == '\\';
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::parse_args(int argc, char* argv[])
{
    cli::Parser parser(argc, argv);

    configure_parser(parser);

    parse_command_line(parser, cmd_line);

    return set_convert_mode(cmd_line, convert_mode);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::convert()
{
    if (convert_mode == CONVERT_ROUTE)
    {
        return convert_route(cmd_line.input_route_path.value,
                             cmd_line.output_route_path.value,
                             cmd_line.input_only_used_at_map.value,
                             cmd_line.input_lights_at_map.value,
                             cmd_line.input_compress_textures.value,
                             cmd_line.num_threads.value);
    }

    if (convert_mode == CONVERT_MODEL)
    {
        return convert_model(cmd_line.input_model_path.value,
                             cmd_line.input_texture_path.value,
                             cmd_line.output_model_path.value,
                             cmd_line.input_compress_textures.value,
                             cmd_line.smooth.value);
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::convert_route(std::string &in_dmd_route_path,
                                std::string &out_gltf_route_path,
                                bool only_used_at_map,
                                bool lights_at_map,
                                bool compress_texture,
                                int num_threads)
{
    // Преобразуем пути к платформоспецифичному виду
    path_to_native_separator(in_dmd_route_path);
    path_to_native_separator(out_gltf_route_path);

    using Label = std::string;
    using RelativeModelPath = std::string;
    using RelativeTexturePath = std::string;
    using LabelTextureSmooth = std::tuple<Label, RelativeTexturePath, bool>;
    std::string line_buffer;

    // Список объектов, используемых в .map маршрута
    std::set<Label> map_objects;

    if (only_used_at_map)
    {
        // Ищем расположение объектов маршрута - файл route1.map
        std::ifstream route1_map = std::ifstream();

        // Сначала пробуем найти конвертированный .map
        std::string map_path = combine_path(out_gltf_route_path, "topology");
        map_path = combine_path(map_path, "map");
        map_path = combine_path(map_path, "route1.map");
        route1_map.open(map_path, std::ios::in);
        if (route1_map.is_open())
        {
            LOG_INFO("Info: opened converted route1.map: %s", map_path.c_str());
        }
        else
        {
            LOG_WARN("Warn: failed to open converted route1.map: %s", map_path.c_str());

            // Пробуем найти оригинальный .map
            map_path = combine_path(in_dmd_route_path, "route1.map");
            route1_map.open(map_path, std::ios::in);
            if (route1_map.is_open())
            {
                LOG_INFO("Info: opened ZDSimulator's route1.map: %s", map_path.c_str());
            }
            else
            {
                LOG_WARN("Warn: failed to open ZDSimulator's route1.map: %s", map_path.c_str());
                return false;
            }
        }

        while (std::getline(route1_map, line_buffer))
        {
            // Пустое название объекта
            if (line_buffer.empty() || (*(line_buffer.begin()) == ',') )
            {
                continue;
            }
            // Строка с объектом должна заканчиваться точкой с запятой
            if (*(line_buffer.end() - 1) != ';')
            {
                continue;
            }
            // Строка с объектом должна содержать шесть запятых - разделителей
            if (std::count(line_buffer.begin(), line_buffer.end(), ',') != 6)
            {
                continue;
            }

            // Читаем первый элемент в строке - сокращённое название объекта
            std::string label = "";
            std::istringstream ss(line_buffer);
            if (std::getline(ss, label, ','))
            {
                map_objects.insert(label);
            }
        }

        if (map_objects.empty())
        {
            LOG_WARN("Warn: failed to find any objects in route1.map");
            route1_map.close();
            return false;
        }

        route1_map.close();
    }

    // Читаем список объектов из базы маршрута
    std::string ref_path = combine_path(in_dmd_route_path, "objects.ref");
    std::ifstream objects_ref(ref_path, std::ios::in);
    if (!objects_ref.is_open())
    {
        LOG_WARN("Warn: failed to open ZDSimulator's objects.ref: %s", ref_path.c_str());
        return false;
    }
    LOG_INFO("Info: opened ZDSimulator's objects.ref: %s", ref_path.c_str());

    // Список моделей, и списки сокращённых имён и текстур к этим моделям
    std::map<RelativeModelPath, std::vector<LabelTextureSmooth>> objects;
    std::set<Label> unique_refs;
    bool light_found = false;
    bool smooth = false;

    while (std::getline(objects_ref, line_buffer))
    {
        std::istringstream ss(line_buffer);
        if (!ss)
        {
            continue;
        }

        // Исходная информация - сокращённое имя, путь к модели, путь к текстуре
        std::string label, relative_dmd_model_path, relative_texture_path;
        ss >> label >> relative_dmd_model_path >> relative_texture_path;
        if (is_slash(label.front())
            || relative_dmd_model_path.empty()
            || !is_slash(relative_dmd_model_path.front())
            || relative_texture_path.empty()
            || !is_slash(relative_texture_path.front()))
        {
            if (label == "[smooth]")
            {
                smooth = true;
                LOG_INFO("Info: smoothing models enabled");
            }
            else if (label == "[not_smooth]")
            {
                smooth = false;
                LOG_INFO("Info: smoothing models disabled");
            }
            continue;
        }

        // Проверяем что сокращённое имя уникально
        if (unique_refs.find(label) == unique_refs.end())
        {
            unique_refs.insert(label);
        }
        else
        {
            LOG_WARN("Warn: ref \"%s\":", line_buffer.c_str());
            LOG_WARN("      name \"%s\" is written already. This ref will be ignored", label.c_str());
            continue;
        }

        // Если прочтён файл .map - проверяем что сокращённое имя используется
        if (only_used_at_map && (map_objects.find(label) == map_objects.end()))
        {
            LOG_WARN("Warn: ref \"%s\":", line_buffer.c_str());
            LOG_WARN("      name \"%s\" does not used at map. Model will not be converted", label.c_str());
            continue;
        }

        // Запоминаем, что нашли модель, на которую ZDS вешает источник света
        if (label == "light")
        {
            light_found = true;
            continue;
        }

        std::replace(relative_dmd_model_path.begin(), relative_dmd_model_path.end(), '\\', '/');
        std::replace(relative_texture_path.begin(), relative_texture_path.end(), '\\', '/');

        auto it = objects.find(relative_dmd_model_path);
        if (it == objects.end())
        {
            // Добавляем файл модели, её сокращённое наименование и текстуру
            objects.insert({ relative_dmd_model_path, {{label, relative_texture_path, smooth}} });
        }
        else
        {
            // К уже добавленой модели ещё вариант наименования и текстуры
            it->second.push_back({label, relative_texture_path, smooth});
            it->second.shrink_to_fit();
        }
    }

    objects_ref.close();

    if (objects.empty())
    {
        LOG_WARN("Warn: failed to find any objects in objects.ref");
        return false;
    }

    // Добавляем в список для конвертации модель неба
    Label skybox_label = "sky";
    RelativeModelPath skybox_model_path = "/models/sky.dmd";
    RelativeTexturePath skybox_texture_path = "/textures/sky_day.bmp";
    objects.insert({ skybox_model_path, {{skybox_label, skybox_texture_path, true}} });

    // Создаем каталог под новый маршрут
    fs::create_directories(out_gltf_route_path);
    // Создаем каталог под текстуры
    fs::create_directory(combine_path(out_gltf_route_path, "textures"));

    // Новый список ссылок на файлы моделей
    std::map<Label, RelativeModelPath> new_objects;
    // Добавляем модель с источником света вместо костыльной модели для ZDS
    if (lights_at_map && light_found)
    {
        new_objects.insert({"light", "/../../data/models/default-objects/light.gltf"});
    }

    uint32_t models_total = objects.size();
    uint32_t models_count = 0;
    auto prev_time = std::chrono::steady_clock::now();
    std::cerr << " (" << 0 << "/" << models_total << ")";

    uint32_t smooth_count = 0;
    uint32_t not_smooth_count = 0;
    for (const auto& [relative_model_path, labels_textures_smooth] : objects)
    {
        fs::path model_path = relative_model_path;

        // Путь к исходной модели
        std::string in_dmd_model_path =
            in_dmd_route_path + relative_model_path;
        path_to_native_separator(in_dmd_model_path);

        // Путь к папке с новым файлом модели
        std::string out_gltf_model_dir =
            out_gltf_route_path + model_path.parent_path().string();

        // Относительный путь к папке с файлами текстур
        std::string mps = model_path.string();
        auto slash_count = std::count(mps.begin(), mps.end(), '/');

        std::string out_relative_texture_dir = "";

        for (int i = 1; i < slash_count; ++i)
        {
            out_relative_texture_dir += "../";
        }
        out_relative_texture_dir += "textures/";

        // Относительный путь к файлу с информацией о модели в формате bin
        fs::create_directories(out_gltf_route_path + model_path.parent_path().string() + "/bin");
        std::string out_relative_bin_path = "bin/"s + model_path.stem().string() + ".bin";

        // Создаём модели для всех вариантов текстур
        bool add_texture_name = (labels_textures_smooth.size() > 1);
        for (const auto& [label, relative_texture_path, smooth] : labels_textures_smooth)
        {
            // Путь к текстуре
            std::string in_texture_path =
                in_dmd_route_path + relative_texture_path;
            path_to_native_separator(in_texture_path);

            fs::path texture_path = relative_texture_path;
            std::string out_relative_texture_path = out_relative_texture_dir;
            if (compress_texture)
            {
                out_relative_texture_path += texture_path.filename().stem().string() + ".ktx2";
            }
            else
            {
                out_relative_texture_path += texture_path.filename().string();
            }

            std::ifstream texture(in_texture_path, std::ios::in);
            if (!texture.is_open())
            {
                LOG_WARN("Warn: failed to open texture file: %s", in_texture_path.c_str());
                LOG_WARN("      model with name \"%s\" will not be converted", label.c_str());
                continue;
            }
            texture.close();

            // Читаем файл модели
            Geometry model_data;
            std::string texture_ext = fs::path(in_texture_path).extension().string();
            model_data.is_reversed_texture_coord = (texture_ext != ".tga");
            model_data.is_blend_material = ((texture_ext == ".tga") || (texture_ext == ".png"));
            if (!get_dmd_model_data(in_dmd_model_path, model_data, smooth))
            {
                LOG_WARN("      model with name \"%s\" will not be converted", label.c_str());
                continue;
            }

            // Путь к новому файлу модели
            std::string out_gltf_model_name = model_path.stem().string();
            if (add_texture_name)
            {
                out_gltf_model_name += '_' + texture_path.stem().string();
            }
            model_data.model_file_name = out_gltf_model_name;

            // Поскольку уровень головки рельса в ZDS маршрутах находится не в нуле,
            // а поднят в среднем на 0.3114, смещаем меш у моделей рельс (с "track" в имени объекта)
            float change_vertices_Z = -0.3114f;
            if (   (label.find("track") == label.npos)
                && (label.find("Track") == label.npos))
            {
                // У всех прочих моделей уже опущена привязка в route1.map
                change_vertices_Z = 0.0f;
            }

            if (generate_gltf_model(model_data,
                                    in_texture_path,
                                    out_gltf_model_dir,
                                    out_relative_bin_path,
                                    out_relative_texture_path,
                                    compress_texture,
                                    change_vertices_Z))
            {
                // Записываем новые модели, кроме неба
                if (label != skybox_label)
                {
                    new_objects.insert({label, model_path.parent_path().string() + '/' + out_gltf_model_name + ".gltf"});
                }

                smooth ? ++smooth_count : ++not_smooth_count;
            }
        }
        ++models_count;
        const auto cur_time = std::chrono::steady_clock::now();
        if (std::chrono::duration<double> diff = cur_time - prev_time; diff.count() > 0.5)
        {
            std::cerr << " (" << models_count << "/" << models_total << ")";
            prev_time = cur_time;
        }
    }
    LOG_INFO("Info: converted %u smooth models and %u not-smooth models", smooth_count, not_smooth_count);

    std::ofstream new_objects_ref(combine_path(out_gltf_route_path, "objects.ref"), std::ios::out);
    if (!new_objects_ref.is_open())
    {
        LOG_WARN("Warn: failed to create new objects.ref: %s", combine_path(out_gltf_route_path, "objects.ref").c_str());
        return false;
    }

    for (const auto& [label, path] : new_objects)
    {
        new_objects_ref << label << "\t" << path << '\n';
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::convert_model(std::string &in_dmd_model_path,
                                std::string &in_texture_path,
                                std::string &out_gltf_model_path,
                                bool smooth,
                                bool compress_texture,
                                std::string out_relative_bin_path,
                                std::string out_relative_texture_path)
{
    Geometry model_data;

    std::ifstream texture(in_texture_path, std::ios::in);
    if (!texture.is_open())
    {
        LOG_WARN("Warn: failed to open texture file: %s", in_texture_path.c_str());
        return false;
    }
    texture.close();

    std::string texture_ext = fs::path(in_texture_path).extension().string();
    model_data.is_reversed_texture_coord = (texture_ext != ".tga");
    model_data.is_blend_material = ((texture_ext == ".tga") || (texture_ext == ".png"));
    if (compress_texture)
    {
        texture_ext = ".ktx2";
    }

    if (!get_dmd_model_data(in_dmd_model_path, model_data, smooth))
    {
        return false;
    }

    fs::path model_path = out_gltf_model_path;
    std::string gltf_directory_path = model_path.parent_path().string();
    fs::create_directories(gltf_directory_path);

    // Относительный путь к файлу с информацией о модели в формате bin
    if (out_relative_bin_path.empty())
    {
        out_relative_bin_path = model_path.stem().string() + ".bin";
    }
    fs::path bin_path = out_relative_bin_path;
    fs::create_directories(bin_path);
    if (bin_path.extension() != ".bin")
    {
        out_relative_bin_path = out_relative_bin_path + ".bin";
    }

    // Относительный путь к текстуре
    if (out_relative_texture_path.empty())
    {
        if (compress_texture)
        {
            out_relative_texture_path = fs::path(in_texture_path).filename().stem().string() + ".ktx2";
        }
        else
        {
            out_relative_texture_path = fs::path(in_texture_path).filename().string();
        }
    }
    fs::path texture_path = out_relative_texture_path;
    fs::create_directories(texture_path);
    if (texture_path.extension() != texture_ext)
    {
        out_relative_texture_path = out_relative_texture_path + texture_ext;
    }

    return generate_gltf_model(model_data,
                               in_texture_path,
                               gltf_directory_path,
                               out_relative_bin_path,
                               out_relative_texture_path,
                               compress_texture,
                               0.0f);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::get_dmd_model_data(std::string &in_dmd_model_path,
                                     Geometry& model_data, bool smooth)
{
    using PosIndex = std::uint32_t;
    using TexIndex = std::uint32_t;
    using VertexIndex = std::uint32_t;

    std::ifstream model_file(in_dmd_model_path, std::ios::in);
    if (!model_file.is_open())
    {
        LOG_WARN("Warn: failed to open model file: %s", in_dmd_model_path.c_str());
        return false;
    }

    std::string file_name = fs::path(in_dmd_model_path).filename().string();
    model_data.model_file_name = file_name.substr(0, file_name.find_last_of('.'));

    std::string buffer;
    while (buffer != "TriMesh()")
    {
        model_file >> buffer;
        if (!model_file)
        {
            LOG_WARN("Warn: failed to find \"TriMesh()\" in file: %s", in_dmd_model_path.c_str());
            return false;
        }
    }

    model_file >> buffer >> buffer;

    std::uint32_t pos_count, pos_face_count;
    model_file >> pos_count >> pos_face_count;

    model_file >> buffer >> buffer;

    std::vector<Vec3> positions(pos_count);
    for (auto& pos : positions)
    {
        model_file >> pos.x >> pos.y >> pos.z;
    }

    if (!model_file)
    {
        LOG_WARN("Warn: failed to read positions from file: %s", in_dmd_model_path.c_str());
    }

    model_file >> buffer >> buffer >> buffer >> buffer;

    std::vector<PosIndex> pos_indices(pos_face_count * 3);
    for (auto& index : pos_indices)
    {
        model_file >> index;
        --index;
    }

    if (!model_file)
    {
        LOG_WARN("Warn: failed to read position indices from file: %s", in_dmd_model_path.c_str());
        return false;
    }

    while (buffer != "Texture:")
    {
        model_file >> buffer;
        if (!model_file)
        {
            LOG_WARN("Warn: failed to find \"Texture:\" in file: %s", in_dmd_model_path.c_str());
            return false;
        }
    }

    model_file >> buffer >> buffer;

    std::uint32_t tex_coord_count, tex_face_count;
    model_file >> tex_coord_count >> tex_face_count;

    if (pos_face_count != tex_face_count)
    {
        LOG_WARN("Warn: position face count is not equal to texture face count in file: %s", in_dmd_model_path.c_str());
        return false;
    }

    std::uint32_t face_count = pos_face_count;

    model_file >> buffer >> buffer;

    std::vector<Vec2> tex_coords(tex_coord_count);
    for (auto& tex_coord : tex_coords)
    {
        model_file >> tex_coord.x >> tex_coord.y >> buffer;

        if (model_data.is_reversed_texture_coord)
        {
            tex_coord.y = 1.0f - tex_coord.y;
        }
    }

    if (!model_file)
    {
        LOG_WARN("Warn: failed to read texture coordinates from file: %s", in_dmd_model_path.c_str());
        return false;
    }

    model_file >> buffer >> buffer >> buffer >> buffer >> buffer;

    std::vector<TexIndex> tex_indices(face_count * 3);
    for (auto& index : tex_indices)
    {
        model_file >> index;
        --index;
    }

    if (!model_file)
    {
        LOG_WARN("Warn: failed to read texture indices from file: %s", in_dmd_model_path.c_str());
        return false;
    }

    model_file.close();


    using Tile_index_t = int;
    using Vertex_tile_t = std::tuple<int, int, int>;
    using Face_unique_t = std::tuple<Vertex_tile_t, Vertex_tile_t, Vertex_tile_t>;
    struct Face_t
    {
        Vec3 vertex[3];
        Vec2 t_coord[3];
        Vec3 normal;
        float normal_length;
        int8_t normal_variant[3] = {0, 0, 0};
        int8_t t_coord_variant[3] = {0, 0, 0};
    };
    std::map<Face_unique_t, Face_t> unique_faces;

    using Vertex_tcoord_variants_t = std::vector<std::pair<Vec2, uint32_t>>;
    using Vertex_normal_variants_t = std::vector<std::pair<Vec3, Vertex_tcoord_variants_t>>;
    std::map<Vertex_tile_t, std::pair<Vec3, Vertex_normal_variants_t>> unique_vertices;

    auto tile = [](const float& coord) -> Tile_index_t
    {
        return static_cast<Tile_index_t>(std::round(coord * 5000.0f));
    };
    auto tile_pos = [&](const Vec3& pos) -> Vertex_tile_t
    {
        return {tile(pos.z), tile(pos.y), tile(pos.x)};
    };

    uint32_t wrong_normals_count = 0;
    uint32_t repeat_faces_count = 0;
    uint32_t reusing_vertex_count = 0;
    for (std::uint32_t i = 0; i < face_count; ++i)
    {
        const PosIndex& pos_index1 = pos_indices[i * 3];
        const PosIndex& pos_index2 = pos_indices[i * 3 + 1];
        const PosIndex& pos_index3 = pos_indices[i * 3 + 2];

        const Vec3& p1 = positions[pos_index1];
        const Vec3& p2 = positions[pos_index2];
        const Vec3& p3 = positions[pos_index3];

        const Vec3 v12 = {p2.x - p1.x, p2.y - p1.y, p2.z - p1.z};
        const Vec3 v13 = {p3.x - p1.x, p3.y - p1.y, p3.z - p1.z};
        Vec3 n = {
            v12.y * v13.z - v12.z * v13.y,
            v12.z * v13.x - v12.x * v13.z,
            v12.x * v13.y - v12.y * v13.x
        };
        float n_length = n.x * n.x + n.y * n.y + n.z * n.z;
        if (n_length < 1e-10)
        {
            //LOG_WARN("Wrong normal (%e) for face %u in file: %s", length, i, in_dmd_model_path.c_str());
            ++wrong_normals_count;
            continue;
        }
        n_length = std::sqrt(n_length);
        //n = {n.x / n_length, n.y / n_length, n.z / n_length};

        Vertex_tile_t vt1 = tile_pos(p1);
        Vertex_tile_t vt2 = tile_pos(p2);
        Vertex_tile_t vt3 = tile_pos(p3);
        Face_unique_t ft = {vt1, vt2, vt3};
        if (vt1 < vt2)
        {
            if (vt1 < vt3)
            {
                ft = {vt1, vt2, vt3};
            }
            else
            {
                ft = {vt3, vt1, vt2};
            }
        }
        else
        {
            if (vt2 < vt3)
            {
                ft = {vt2, vt3, vt1};
            }
            else
            {
                ft = {vt3, vt1, vt2};
            }
        }

        if (unique_faces.find(ft) == unique_faces.end())
        {
            const TexIndex& tex_index1 = tex_indices[i * 3];
            const TexIndex& tex_index2 = tex_indices[i * 3 + 1];
            const TexIndex& tex_index3 = tex_indices[i * 3 + 2];

            const Vec2& t_coord1 = tex_coords[tex_index1];
            const Vec2& t_coord2 = tex_coords[tex_index2];
            const Vec2& t_coord3 = tex_coords[tex_index3];

            Face_t new_face = {p1, p2, p3,
                               t_coord1, t_coord2, t_coord3,
                               n, n_length};

            if (smooth)
            {
                auto find_or_create_unique_vertex_variant = [](Face_t& _face,
                    std::map<Vertex_tile_t, std::pair<Vec3, Vertex_normal_variants_t>>& _unique_vertices,
                    Vertex_tile_t& _vt, uint8_t _v_idx, uint32_t& _reusing_vertex_count)
                {
                    Vec3& _p = _face.vertex[_v_idx];
                    Vec3& _n = _face.normal;
                    Vec2& _t_coord = _face.t_coord[_v_idx];

                    auto it = _unique_vertices.find(_vt);
                    if (it == _unique_vertices.end())
                    {
                        _face.normal_variant[_v_idx] = 0;
                        _face.t_coord_variant[_v_idx] = 0;
                        _unique_vertices.insert({ _vt, {_p, {{ _n, {{_t_coord, 0}} }} } });
                    }
                    else
                    {
                        Vertex_normal_variants_t& vnv = it->second.second;
                        bool add_new_normal_variant = true;
                        for (uint8_t n_var = 0; n_var < vnv.size(); ++n_var)
                        {
                            Vec3& nv = vnv[n_var].first;
                            const float nv_length = std::sqrt(nv.x * nv.x + nv.y * nv.y + nv.z * nv.z);
                            const float n_nv_dot = _n.x * nv.x + _n.y * nv.y + _n.z * nv.z;
                            const float cos_n_nv = n_nv_dot / (_face.normal_length * nv_length);
                            if (cos_n_nv > 0.7)
                            {
                                _face.normal_variant[_v_idx] = n_var;
                                add_new_normal_variant = false;

                                nv.x += _n.x;
                                nv.y += _n.y;
                                nv.z += _n.z;

                                Vertex_tcoord_variants_t& vtcv = vnv[n_var].second;
                                bool add_new_tcoord_variant = true;
                                for (uint8_t tc_var = 0; tc_var < vtcv.size(); ++tc_var)
                                {
#if 1
                                    if (   (std::abs(_t_coord.x - vtcv[tc_var].first.x) < 1e-5f)
                                        && (std::abs(_t_coord.y - vtcv[tc_var].first.y) < 1e-5f))
#else
                                    if (   (_t_coord.x == vtcv[tc_var].first.x)
                                        && (_t_coord.y == vtcv[tc_var].first.y))
#endif
                                    {
                                        _face.t_coord_variant[_v_idx] = tc_var;
                                        add_new_tcoord_variant = false;
                                        ++_reusing_vertex_count;
                                        break;
                                    }
                                }
                                if (add_new_tcoord_variant)
                                {
                                    vtcv.push_back({_t_coord, 0});
                                }

                                break;
                            }
                        }
                        if (add_new_normal_variant)
                        {
                            _face.normal_variant[_v_idx] = vnv.size();
                            _face.t_coord_variant[_v_idx] = 0;
                            vnv.push_back({ _n, {{_t_coord, 0}} });
                        }
                    }
                };

                find_or_create_unique_vertex_variant(
                    new_face, unique_vertices, vt1, 0,reusing_vertex_count);
                find_or_create_unique_vertex_variant(
                    new_face, unique_vertices, vt2, 1,reusing_vertex_count);
                find_or_create_unique_vertex_variant(
                    new_face, unique_vertices, vt3, 2,reusing_vertex_count);
            }

            unique_faces.insert({ft, new_face});
        }
        else
        {
            //LOG_WARN("Face %u is already exists in file: %s", i, in_dmd_model_path.c_str());
            ++repeat_faces_count;
            continue;
        }
    }
    if (wrong_normals_count)
    {
        LOG_WARN("Excluded %u faces with wrong normal in file: %s", wrong_normals_count, in_dmd_model_path.c_str());
    }
    if (repeat_faces_count)
    {
        LOG_WARN("Excluded %u repeat faces in file: %s", repeat_faces_count, in_dmd_model_path.c_str());
    }

    model_data.vertices.reserve(unique_faces.size() * 3);
    if (smooth)
    {
        for (auto& [vt, pos_and_normal_variants] : unique_vertices)
        {
            Vec3& pos = pos_and_normal_variants.first;
            for (auto& [nv, t_coord_variants] : pos_and_normal_variants.second)
            {
                const float nv_length_inv = 1.0f / std::sqrt(nv.x * nv.x + nv.y * nv.y + nv.z * nv.z);
                nv = {nv.x * nv_length_inv, nv.y * nv_length_inv, nv.z * nv_length_inv};

                for (auto& [t_coord, index] : t_coord_variants)
                {
                    index = model_data.vertices.size();
                    model_data.vertices.emplace_back(Vertex{pos, nv, t_coord});
                }
            }
        }

        const bool indices16 = (model_data.vertices.size() < 65535);
        if (indices16)
        {
            model_data.indices16.reserve(unique_faces.size() * 3);
        }
        else
        {
            model_data.indices32.reserve(unique_faces.size() * 3);
        }

        auto vertex_index = [](Vertex_tile_t _vt, uint8_t _normal_variant, uint8_t _tcoord_variant,
            std::map<Vertex_tile_t, std::pair<Vec3, Vertex_normal_variants_t>>& _unique_vertices) -> uint32_t
        {
            auto it = _unique_vertices.find(_vt);
            Vertex_normal_variants_t& vnv = it->second.second;
            Vertex_tcoord_variants_t& vtcv = vnv[_normal_variant].second;
            return vtcv[_tcoord_variant].second;
        };

        for (auto& [ft, face] : unique_faces)
        {
            Vertex_tile_t vt1 = tile_pos(face.vertex[0]);
            Vertex_tile_t vt2 = tile_pos(face.vertex[1]);
            Vertex_tile_t vt3 = tile_pos(face.vertex[2]);

            uint32_t index1 = vertex_index(vt1, face.normal_variant[0], face.t_coord_variant[0], unique_vertices);
            uint32_t index2 = vertex_index(vt2, face.normal_variant[1], face.t_coord_variant[1], unique_vertices);
            uint32_t index3 = vertex_index(vt3, face.normal_variant[2], face.t_coord_variant[2], unique_vertices);
            if (indices16)
            {
                model_data.indices16.push_back(index1);
                model_data.indices16.push_back(index2);
                model_data.indices16.push_back(index3);
            }
            else
            {
                model_data.indices32.push_back(index1);
                model_data.indices32.push_back(index2);
                model_data.indices32.push_back(index3);
            }
        }
    }
    else
    {
        for (const auto& [ft, face] : unique_faces)
        {
            const float n_length_inv = 1.0f / face.normal_length;
            const Vec3 normal = {face.normal.x * n_length_inv,
                                 face.normal.y * n_length_inv,
                                 face.normal.z * n_length_inv};
            model_data.vertices.emplace_back(Vertex{face.vertex[0], normal, face.t_coord[0]});
            model_data.vertices.emplace_back(Vertex{face.vertex[1], normal, face.t_coord[1]});
            model_data.vertices.emplace_back(Vertex{face.vertex[2], normal, face.t_coord[2]});
        }
    }
    model_data.vertices.shrink_to_fit();
/*
    if (reusing_vertex_count)
    {
        LOG_INFO("Info: %u vertices (reusing %u/%u) in model from file: %s", model_data.vertices.size(), reusing_vertex_count, unique_faces.size() * 3, in_dmd_model_path.c_str());
    }
*/
    return (model_data.vertices.size() > 0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::generate_gltf_model(Geometry& model_data,
                                      std::string &in_texture_path,
                                      std::string &gltf_directory_path,
                                      std::string &out_relative_bin_path,
                                      std::string &out_relative_texture_path,
                                      bool compress_texture,
                                      float change_vertices_Z)
{
    for (auto& vertex : model_data.vertices)
    {
        std::swap(vertex.pos.y, vertex.pos.z);
        vertex.pos.y += change_vertices_Z;
        vertex.pos.z = -vertex.pos.z;

        std::swap(vertex.normal.y, vertex.normal.z);
        vertex.normal.z = -vertex.normal.z;
    }

    std::string full_bin_path = combine_path(gltf_directory_path, out_relative_bin_path);

    std::ofstream bin_file(full_bin_path, std::ios::binary | std::ios::out);
    if (!bin_file.is_open())
    {
        LOG_WARN("Warn: failed to create and open bin file: %s", full_bin_path.c_str());
        return false;
    }

    for (const auto& vertex : model_data.vertices)
    {
        bin_file.write(reinterpret_cast<const char*>(&vertex.pos), sizeof(vertex.pos));
    }

    auto positions_byte_length = bin_file.tellp();

    for (const auto& vertex : model_data.vertices)
    {
        bin_file.write(reinterpret_cast<const char*>(&vertex.normal), sizeof(vertex.normal));
    }

    auto normals_byte_length = bin_file.tellp() - positions_byte_length;

    for (const auto& vertex : model_data.vertices)
    {
        bin_file.write(reinterpret_cast<const char*>(&vertex.tex_coord), sizeof(vertex.tex_coord));
    }

    auto tex_coords_byte_length = bin_file.tellp() - normals_byte_length - positions_byte_length;

    std::uint32_t indices_count = 0;
    std::string indices_buffer_componentType = "";
    if (model_data.indices16.size())
    {
        indices_buffer_componentType = "5123";
        indices_count = model_data.indices16.size();
        for (auto index16 : model_data.indices16)
        {
            bin_file.write(reinterpret_cast<const char*>(&index16), sizeof(index16));
        }
    }
    else if (model_data.indices32.size())
    {
        indices_buffer_componentType = "5125";
        indices_count = model_data.indices32.size();
        for (auto index32 : model_data.indices32)
        {
            bin_file.write(reinterpret_cast<const char*>(&index32), sizeof(index32));
        }
    }

    auto indices_byte_length = bin_file.tellp() - tex_coords_byte_length - normals_byte_length - positions_byte_length;

    bin_file.close();

    Vec3 min_pos, max_pos;
    Vec3 min_norm, max_norm;
    Vec2 min_tex, max_tex;
    min_pos = max_pos = model_data.vertices[0].pos;
    min_norm = max_norm = model_data.vertices[0].normal;
    min_tex = max_tex = model_data.vertices[0].tex_coord;

    for (const auto& vertex : model_data.vertices)
    {
        min_pos.x = std::min(min_pos.x, vertex.pos.x);
        min_pos.y = std::min(min_pos.y, vertex.pos.y);
        min_pos.z = std::min(min_pos.z, vertex.pos.z);

        max_pos.x = std::max(max_pos.x, vertex.pos.x);
        max_pos.y = std::max(max_pos.y, vertex.pos.y);
        max_pos.z = std::max(max_pos.z, vertex.pos.z);

        min_norm.x = std::min(min_norm.x, vertex.normal.x);
        min_norm.y = std::min(min_norm.y, vertex.normal.y);
        min_norm.z = std::min(min_norm.z, vertex.normal.z);

        max_norm.x = std::max(max_norm.x, vertex.normal.x);
        max_norm.y = std::max(max_norm.y, vertex.normal.y);
        max_norm.z = std::max(max_norm.z, vertex.normal.z);

        min_tex.x = std::min(min_tex.x, vertex.tex_coord.x);
        min_tex.y = std::min(min_tex.y, vertex.tex_coord.y);

        max_tex.x = std::max(max_tex.x, vertex.tex_coord.x);
        max_tex.y = std::max(max_tex.y, vertex.tex_coord.y);
    }

    std::string gltf_path = combine_path(gltf_directory_path, model_data.model_file_name + ".gltf");

    std::ofstream gltf_file(gltf_path, std::ios::out);
    if (!gltf_file.is_open())
    {
        LOG_WARN("Warn: failed to create and open gltf file: %s", gltf_path.c_str());
        return false;
    }

    std::ostringstream indices_bufferView;
    std::ostringstream indices_accessor;
    std::string primitives_indices = "";
    if (indices_count)
    {
        indices_bufferView << ",\n"
            "        {\n"
            "            \"buffer\": 0,\n"
            "            \"byteOffset\": " << positions_byte_length + normals_byte_length + tex_coords_byte_length << ",\n"
            "            \"byteLength\": " << indices_byte_length << ",\n"
            "            \"target\": 34963\n"
            "        }";

        indices_accessor << ",\n"
            "        {\n"
            "            \"bufferView\": 3,\n"
            "            \"componentType\": "<< indices_buffer_componentType << ",\n"
            "            \"count\": " << indices_count << ",\n"
            "            \"type\": \"SCALAR\",\n"
            "            \"max\": [\n"
            "                " << indices_count - 1 << "\n"
            "            ],\n"
            "            \"min\": [\n"
            "                0\n"
            "            ]\n"
            "        }";

        primitives_indices = ",\n                    \"indices\": 3";
    }

    std::string blend = "";
    if (model_data.is_blend_material)
    {
        blend = ",\n            \"alphaMode\": \"BLEND\"";
    }

    gltf_file << "{\n"
        "    \"asset\": {\n"
        "        \"generator\": \"dmd2gltf\",\n"
        "        \"version\": \"2.0\"\n"
        "    },\n"
        "    \"buffers\": [\n"
        "        {\n"
        "            \"uri\": \"" << out_relative_bin_path << "\",\n"
        "            \"byteLength\": " << positions_byte_length + normals_byte_length + tex_coords_byte_length + indices_byte_length << "\n"
        "        }\n"
        "    ],\n"
        "    \"bufferViews\": [\n"
        "        {\n"
        "            \"buffer\": 0,\n"
        "            \"byteOffset\": 0,\n"
        "            \"byteLength\": " << positions_byte_length << ",\n"
        "            \"target\": 34962\n"
        "        },\n"
        "        {\n"
        "            \"buffer\": 0,\n"
        "            \"byteOffset\": " << positions_byte_length << ",\n"
        "            \"byteLength\": " << normals_byte_length << ",\n"
        "            \"target\": 34962\n"
        "        },\n"
        "        {\n"
        "            \"buffer\": 0,\n"
        "            \"byteOffset\": " << positions_byte_length + normals_byte_length << ",\n"
        "            \"byteLength\": " << tex_coords_byte_length << ",\n"
        "            \"target\": 34962\n"
        "        }" << indices_bufferView.str() << "\n"
        "    ],\n"
        "    \"accessors\": [\n"
        "        {\n"
        "            \"bufferView\": 0,\n"
        "            \"componentType\": 5126,\n"
        "            \"count\": " << model_data.vertices.size() << ",\n"
        "            \"type\": \"VEC3\",\n"
        "            \"max\": [\n"
        "                " << max_pos.x << ",\n"
        "                " << max_pos.y << ",\n"
        "                " << max_pos.z << "\n"
        "            ],\n"
        "            \"min\": [\n"
        "                " << min_pos.x << ",\n"
        "                " << min_pos.y << ",\n"
        "                " << min_pos.z << "\n"
        "            ]\n"
        "        },\n"
        "        {\n"
        "            \"bufferView\": 1,\n"
        "            \"componentType\": 5126,\n"
        "            \"count\": " << model_data.vertices.size() << ",\n"
        "            \"type\": \"VEC3\",\n"
        "            \"max\": [\n"
        "                " << max_norm.x << ",\n"
        "                " << max_norm.y << ",\n"
        "                " << max_norm.z << "\n"
        "            ],\n"
        "            \"min\": [\n"
        "                " << min_norm.x << ",\n"
        "                " << min_norm.y << ",\n"
        "                " << min_norm.z << "\n"
        "            ]\n"
        "        },\n"
        "        {\n"
        "            \"bufferView\": 2,\n"
        "            \"componentType\": 5126,\n"
        "            \"count\": " << model_data.vertices.size() << ",\n"
        "            \"type\": \"VEC2\",\n"
        "            \"max\": [\n"
        "                " << max_tex.x << ",\n"
        "                " << max_tex.y << "\n"
        "            ],\n"
        "            \"min\": [\n"
        "                " << min_tex.x << ",\n"
        "                " << min_tex.y << "\n"
        "            ]\n"
        "        }" << indices_accessor.str() << "\n"
        "    ],\n"
        "    \"images\": [\n"
        "        {\n"
        "            \"uri\": \"" << out_relative_texture_path << "\"\n"
        "        }\n"
        "    ],\n"
        "    \"samplers\": [\n"
        "        {\n"
        "            \"magFilter\": 9729,\n"
        "            \"minFilter\": 9987,\n"
        "            \"wrapS\": 10497,\n"
        "            \"wrapT\": 10497\n"
        "        }\n"
        "    ],\n"
        "    \"textures\": [\n"
        "        {\n"
        "            \"sampler\": 0,\n"
        "            \"source\": 0\n"
        "        }\n"
        "    ],\n"
        "    \"materials\": [\n"
        "        {\n"
        "            \"pbrMetallicRoughness\": {\n"
        "                \"baseColorTexture\": {\n"
        "                    \"index\": 0,\n"
        "                    \"texCoord\": 0\n"
        "                }\n"
        "            }" << blend << "\n"
        "        }\n"
        "    ],\n"
        "    \"meshes\": [\n"
        "        {\n"
        "            \"primitives\": [\n"
        "                {\n"
        "                    \"attributes\": {\n"
        "                        \"POSITION\": 0,\n"
        "                        \"NORMAL\": 1,\n"
        "                        \"TEXCOORD_0\": 2\n"
        "                    }" << primitives_indices << ",\n"
        "                    \"material\": 0,\n"
        "                    \"mode\": 4\n"
        "                }\n"
        "            ]\n"
        "        }\n"
        "    ],\n"
        "    \"nodes\": [\n"
        "        {\n"
        "            \"mesh\": 0\n"
        "        }\n"
        "    ],\n"
        "    \"scenes\": [\n"
        "        {\n"
        "            \"nodes\": [ 0 ]\n"
        "        }\n"
        "    ],\n"
        "    \"scene\": 0\n"
        "}\n";

    gltf_file.close();

    std::string out_texture_path = gltf_directory_path + '/' + out_relative_texture_path;
    if (!std::filesystem::exists(out_texture_path))
    {
        try
        {
            if (compress_texture)
            {
                compress_to_ktx2(in_texture_path, out_texture_path);
            }
            else
            {
                std::filesystem::copy(in_texture_path, out_texture_path);
            }
        }
        catch (std::exception &e)
        {
            LOG_INFO("Info: %s", e.what());
        }
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::compress_to_ktx2(const std::string& in_texture_path, const std::string& out_texture_path)
{
    int w = 0, h = 0, ch = 0;
    stbi_uc* data = stbi_load(in_texture_path.c_str(), &w, &h, &ch, 4);
    if (!data) {
        LOG_WARN("STBI: error (%s) while loading texture file: %s", stbi_failure_reason(), in_texture_path.c_str());
        return false;
    }

    size_t size = w * h * 4;

    ktxTextureCreateInfo ci = {};
    // --format R8G8B8A8_SRGB
    ci.vkFormat        = VK_FORMAT_R8G8B8A8_SRGB;

    ci.baseWidth       = static_cast<ktx_uint32_t>(w);
    ci.baseHeight      = static_cast<ktx_uint32_t>(h);
    ci.baseDepth       = 1;
    ci.numDimensions   = 2;
    ci.numLayers       = 1;
    ci.numFaces        = 1;
    ci.isArray         = KTX_FALSE;

    // Число mipmap-уровней определяем ЯВНО, так как сами их сгенерируем
    ci.numLevels       = static_cast<uint32_t>(std::floor(std::log2(std::max(w, h))) + 1);
    // Не полагаемся на аппратную генерацию - это для несжатых текстур
    ci.generateMipmaps = KTX_FALSE;

    ktxTexture2* Ktexture2 = nullptr;
    KTX_error_code result = ktxTexture2_Create(&ci,
                                               KTX_TEXTURE_CREATE_ALLOC_STORAGE,
                                               &Ktexture2);

    if (result != KTX_SUCCESS)
    {
        LOG_WARN("KTX: error (%u) while creating ktxTexture2 for file: %s", result, in_texture_path.c_str());
        stbi_image_free(data);
        return false;
    }

    ktxTexture* Ktexture = ktxTexture(Ktexture2);
    result = ktxTexture_SetImageFromMemory(Ktexture,
                                           0, 0, 0,
                                           data, size);
    if (result != KTX_SUCCESS)
    {
        LOG_WARN("KTX: error (%u) to set 0 level of image from file: %s", result, in_texture_path.c_str());
        ktxTexture_Destroy(Ktexture);
        stbi_image_free(data);
        return false;
    }

    // Массив под указатели на данные всех уровней, не удаляем эти данные
    // до сжатия и сохранения текстуры на диск!
    std::vector<stbi_uc *> levels_data = {data};

    // Вручную генерируем mipmaps, последовательно уменьшая
    // размеры текстуры в два раза на каждом уровне
    for (uint32_t level = 1; level < ci.numLevels; ++level)
    {
        int next_w = std::max(1, w >> 1);
        int next_h = std::max(1, h >> 1);
        size = next_w * next_h * 4;
        stbi_uc* next_resized = new stbi_uc[size];

        // Ресемплинг
        stbir_resize_uint8(levels_data.back(), w, h, 0,
                           next_resized, next_w, next_h, 0, 4);
        levels_data.push_back(next_resized);

        // Загружаем уровень в ktxTexture
        result = ktxTexture_SetImageFromMemory(Ktexture,
                                               level, 0, 0,
                                               next_resized, size);
        if (result != KTX_SUCCESS)
        {
            LOG_WARN("KTX: error (%u) to set %u level of image from file: %s", result, level, in_texture_path.c_str());
            ktxTexture_Destroy(Ktexture);
            for (auto d : levels_data)
            {
                stbi_image_free(d);
            }
            return false;
        }
        w = next_w;
        h = next_h;
    }

    ktxBasisParams params = {};
    params.structSize = sizeof(ktxBasisParams);  // Всегда первым полем

    // --encode basis-lz
    params.codec = KTX_BASIS_CODEC_ETC1S;

    // --qlevel (по умолчанию 128)
    params.qualityLevel = 128;

    // --clevel (CLI по умолчанию 1, хотя в libktx константа = 2)
    params.etc1sCompressionLevel = 1;

    // Потоки (по умолчанию 0 = аппаратное определение всех ядер)
    params.threadCount = std::thread::hardware_concurrency();

    // RDO-пороги (по умолчанию 1.25, применяются при qlevel <= 128)
    params.endpointRDOThreshold = 1.25f;
    params.selectorRDOThreshold = 1.25f;

    // Автоматический выбор кластеров (по умолчанию)
    params.maxEndpoints = 0;
    params.maxSelectors = 0;

    // Флаги по умолчанию
    params.verbose           = KTX_FALSE;
    params.noSSE             = KTX_FALSE;
    params.normalMap         = KTX_FALSE;
    params.preSwizzle        = KTX_FALSE;
    params.noEndpointRDO     = KTX_FALSE;
    params.noSelectorRDO     = KTX_FALSE;

    result = ktxTexture2_CompressBasisEx(Ktexture2, &params);

    if (result != KTX_SUCCESS)
    {
        LOG_WARN("KTX: error (%u) while compressing image from file: %s", result, in_texture_path.c_str());
    }
    else
    {
        result = ktxTexture_WriteToNamedFile(Ktexture, out_texture_path.c_str());

        if (result != KTX_SUCCESS)
        {
            LOG_WARN("KTX: error (%u) while writing file: %s", result, out_texture_path.c_str());
        }
    }

    // Освобождаем память от ktx-параметров и от пикселей всех уровней
    ktxTexture_Destroy(Ktexture);
    for (auto d : levels_data)
    {
        stbi_image_free(d);
    }

    return result == KTX_SUCCESS;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Application::configure_parser(cli::Parser &parser)
{
    parser.set_optional<std::string>("i", "input-route",
                                     "",
                                     "Input DMD route path");

    parser.set_optional<bool>("u", "used-only",
                              false,
                              "Convert only models used at map");

    parser.set_optional<bool>("l", "lights",
                              false,
                              "Convert lights at map");

    parser.set_optional<bool>("c", "compress-textures",
                              false,
                              "Convert textures to compressed .ktx2");

    parser.set_optional<bool>("s", "smooth",
                              false,
                              "Smooth normals (single-model use)");

    parser.set_optional<int>("n", "num-threads",
                             1,
                             "Count of threads (route use)");

    parser.set_optional<std::string>("o", "output-route",
                                     "",
                                     "Output GLTF route path");

    parser.set_optional<std::string>("m", "input-model",
                                     "",
                                     "Input DMD model path");

    parser.set_optional<std::string>("t", "input-texture",
                                     "",
                                     "Input DMD texture path");

    parser.set_optional<std::string>("g", "output-model",
                                     "",
                                     "Output GLTF model path");

    parser.enable_help();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Application::parse_command_line(cli::Parser &parser, cmd_line_t &cmd_line)
{
    parser.run_and_exit_if_error();
    cmd_line.input_route_path = parser.get<std::string>("i");
    cmd_line.input_only_used_at_map = parser.get<bool>("u");
    cmd_line.input_lights_at_map = parser.get<bool>("l");
    cmd_line.input_compress_textures = parser.get<bool>("c");
    cmd_line.smooth = parser.get<bool>("s");
    cmd_line.num_threads = parser.get<int>("n");
    cmd_line.output_route_path = parser.get<std::string>("o");
    cmd_line.input_model_path = parser.get<std::string>("m");
    cmd_line.input_texture_path = parser.get<std::string>("t");
    cmd_line.output_model_path = parser.get<std::string>("g");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::set_convert_mode(const cmd_line_t &cmd_line,
                                   ConvertMode &convert_mode)
{
    if (cmd_line.input_route_path.isPresent())
    {
        if (cmd_line.output_route_path.isPresent())
        {
            convert_mode = CONVERT_ROUTE;
            return true;
        }
        else
        {
            LOG_WARN("ERROR: Missing route output path");
            return false;
        }
    }

    if (cmd_line.input_model_path.isPresent())
    {
        if (cmd_line.input_texture_path.isPresent())
        {
            if (cmd_line.output_model_path.isPresent())
            {
                convert_mode = CONVERT_MODEL;
                return true;
            }
            else
            {
                LOG_WARN("ERROR: Missing output GLTF model path");
            }
        }
        else
        {
            LOG_WARN("ERROR: Missing input DMD texture path");
        }
    }
    else
    {
        LOG_WARN("ERROR: Missing input route path or DMD model path");
    }

    return false;
}
