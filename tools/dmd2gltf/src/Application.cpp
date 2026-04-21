#include    <Application.h>
#include    <Geometry.h>
#include    <filesystem-utils.h>
#include    <Logger.h>

#include    <algorithm>
#include    <cmath>
#include    <cstdint>
#include    <filesystem>
#include    <fstream>
#include    <iostream>
#include    <map>
#include    <set>
#include    <string>
#include    <utility>
#include    <vector>

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
                             cmd_line.input_lights_at_map.value);
    }

    if (convert_mode == CONVERT_MODEL)
    {
        return convert_model(cmd_line.input_model_path.value,
                             cmd_line.input_texture_path.value,
                             cmd_line.output_model_path.value);
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::convert_route(std::string &in_dmd_route_path,
                                std::string &out_gltf_route_path,
                                bool only_used_at_map,
                                bool lights_at_map)
{
    // Преобразуем пути к платформоспецифичному виду
    path_to_native_separator(in_dmd_route_path);
    path_to_native_separator(out_gltf_route_path);

    using Label = std::string;
    using RelativeModelPath = std::string;
    using RelativeTexturePath = std::string;
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
    std::map<RelativeModelPath, std::map<Label, RelativeTexturePath>> objects;
    std::set<Label> unique_refs;
    bool light_found = false;

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
            objects.insert({relative_dmd_model_path, {{label, relative_texture_path}} });
        }
        else
        {
            auto it2 = it->second.find(label);
            if (it2 == it->second.end())
            {
                // К уже добавленой модели ещё вариант наименования и текстуры
                it->second.insert({label, relative_texture_path});
            }
            else
            {
                LOG_WARN("Warn: ref \"%s\":", line_buffer.c_str());
                LOG_WARN("      this combination of model and texture is written several times. This ref will be ignored", label.c_str());
            }
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
    objects.insert({ skybox_model_path, {{skybox_label, skybox_texture_path}} });

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

    for (const auto& [relative_model_path, labels_textures] : objects)
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
        bool add_texture_name = (labels_textures.size() > 1);
        for (const auto& [label, relative_texture_path] : labels_textures)
        {
            // Путь к текстуре
            std::string in_texture_path =
                in_dmd_route_path + relative_texture_path;
            path_to_native_separator(in_texture_path);

            fs::path texture_path = relative_texture_path;
            std::string out_relative_texture_path =
                out_relative_texture_dir + texture_path.filename().string();

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
            if (!get_dmd_model_data(in_dmd_model_path, model_data))
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
                                    change_vertices_Z))
            {
                // Записываем новые модели, кроме неба
                if (label != skybox_label)
                {
                    new_objects.insert({label, model_path.parent_path().string() + '/' + out_gltf_model_name + ".gltf"});
                }
            }
        }
    }

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

    if (!get_dmd_model_data(in_dmd_model_path, model_data))
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
        out_relative_texture_path = fs::path(in_texture_path).filename().string();
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
                               out_relative_texture_path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::get_dmd_model_data(std::string &in_dmd_model_path,
                                     Geometry& model_data)
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

    model_data.vertices.reserve(face_count * 3);

    using Tile_index_t = int;
    using Vertex_tile_t = std::tuple<int, int, int>;
    using Face_t = std::tuple<Vertex_tile_t, Vertex_tile_t, Vertex_tile_t>;
    std::set<Face_t> unique_faces;
    auto tile = [](const float& coord) -> Tile_index_t
    {
        return static_cast<Tile_index_t>(std::round(coord * 1000.0f));
    };
    uint32_t wrong_normals_count = 0;
    uint32_t repeat_faces_count = 0;
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
        float length = std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
        if (length < 1e-5)
        {
            //LOG_WARN("Wrong normal (%e) for face %u in file: %s", length, i, in_dmd_model_path.c_str());
            ++wrong_normals_count;
            continue;
        }
        n = {n.x / length, n.y / length, n.z / length};

        Vertex_tile_t vt1 = {tile(p1.x), tile(p1.y), tile(p1.z)};
        Vertex_tile_t vt2 = {tile(p2.x), tile(p2.y), tile(p2.z)};
        Vertex_tile_t vt3 = {tile(p3.x), tile(p3.y), tile(p3.z)};
        Face_t ft;
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
            unique_faces.insert(ft);

            const TexIndex& tex_index1 = tex_indices[i * 3];
            const TexIndex& tex_index2 = tex_indices[i * 3 + 1];
            const TexIndex& tex_index3 = tex_indices[i * 3 + 2];

            model_data.vertices.emplace_back(Vertex{p1, n, tex_coords[tex_index1]});
            model_data.vertices.emplace_back(Vertex{p2, n, tex_coords[tex_index2]});
            model_data.vertices.emplace_back(Vertex{p3, n, tex_coords[tex_index3]});
        }
        else
        {
            //LOG_WARN("Face %u is already exists in file: %s", i, in_dmd_model_path.c_str());
            ++repeat_faces_count;
            continue;
        }
    }
    LOG_WARN("Excluded %u faces with wrong normal in file: %s", wrong_normals_count, in_dmd_model_path.c_str());
    LOG_WARN("Excluded %u repeat faces in file: %s", repeat_faces_count, in_dmd_model_path.c_str());

    model_data.vertices.shrink_to_fit();

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
        "            \"byteLength\": " << positions_byte_length + normals_byte_length  + tex_coords_byte_length << "\n"
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
        "        }\n"
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
        "        }\n"
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
        "                    },\n"
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

    if (!std::filesystem::exists(gltf_directory_path + '/' + out_relative_texture_path))
    {
        try
        {
            std::filesystem::copy(in_texture_path, gltf_directory_path + '/' + out_relative_texture_path);
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
