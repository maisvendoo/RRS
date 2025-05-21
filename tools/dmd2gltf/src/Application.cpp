#include    <Application.h>
#include    <Geometry.h>
#include    <filesystem-utils.h>

#include    <algorithm>
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
                             cmd_line.input_only_used_at_map.isPresent());
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
                                bool only_used_at_map)
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
            std::cout << "Info: opened converted route1.map: " << map_path << std::endl;
        }
        else
        {
            std::cout << "Warn: failed to open converted route1.map: " << map_path << std::endl;

            // Пробуем найти оригинальный .map
            map_path = combine_path(in_dmd_route_path, "route1.map");
            route1_map.open(map_path, std::ios::in);
            if (route1_map.is_open())
            {
                std::cout << "Info: opened route1.map: " << map_path << std::endl;
            }
            else
            {
                std::cerr << "Failed to open route1.map: " << map_path << std::endl;
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
            std::cerr << "Failed to find objects in route1.map" << std::endl;
            route1_map.close();
            return false;
        }

        route1_map.close();
    }

    // Читаем список объектов из базы маршрута
    std::ifstream objects_ref(combine_path(in_dmd_route_path, "objects.ref"), std::ios::in);
    if (!objects_ref.is_open())
    {
        std::cerr << "Failed to open objects.ref" << std::endl;
        return false;
    }
    std::cout << "Info: opened objects.ref: " << combine_path(in_dmd_route_path, "objects.ref") << std::endl;

    // Список моделей, и списки сокращённых имён и текстур к этим моделям
    std::map<RelativeModelPath, std::map<Label, RelativeTexturePath>> objects;

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
        if (is_slash(label.front()) || !is_slash(relative_dmd_model_path.front()) || !is_slash(relative_texture_path.front()))
        {
            continue;
        }

        // Если прочтён файл .map - проверяем что сокращённое имя используется
        if (only_used_at_map && (map_objects.find(label) == map_objects.end()))
        {
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
        }
    }

    objects_ref.close();

    if (objects.empty())
    {
        std::cerr << "Failed to find objects in objects.ref" << std::endl;
        return false;
    }

    // Создаем каталог под новый маршрут
    fs::create_directories(out_gltf_route_path);
    // Создаем каталог под текстуры
    fs::create_directory(combine_path(out_gltf_route_path, "textures"));

    if (in_dmd_route_path != out_gltf_route_path)
    {
        // Копируем топологию
        try
        {
            fs::copy(combine_path(in_dmd_route_path, "topology"),
                     combine_path(out_gltf_route_path, "topology"),
                     fs::copy_options::overwrite_existing | fs::copy_options::recursive);
        }
        catch (std::exception &e)
        {
            std::cerr << e.what();
        }

        try
        {
            fs::copy(combine_path(in_dmd_route_path, "description.xml"),
                     combine_path(out_gltf_route_path, "description.xml"),
                     fs::copy_options::overwrite_existing | fs::copy_options::recursive);
        }
        catch (std::exception &e)
        {
            std::cerr << e.what();
        }
    }

    std::map<Label, RelativeModelPath> new_objects;

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
                std::cerr << "Failed to open " << in_texture_path << std::endl;
                continue;
            }

            // Читаем файл модели
            Geometry model_data;
            std::string texture_ext = fs::path(in_texture_path).extension().string();
            model_data.is_TGA_texture = texture_ext == ".tga";
            if (!get_dmd_model_data(in_dmd_model_path, model_data))
            {
                std::cerr << "Failed to open " << in_dmd_model_path << std::endl;
                continue;
            }

            // Путь к новому файлу модели
            std::string out_gltf_model_name = model_path.stem().string();
            if (add_texture_name)
            {
                out_gltf_model_name += '_' + texture_path.stem().string();
            }
            model_data.model_file_name = out_gltf_model_name;

            if (generate_gltf_model(model_data,
                                    in_texture_path,
                                    out_gltf_model_dir,
                                    out_relative_bin_path,
                                    out_relative_texture_path))
            {
                new_objects.insert({label, model_path.parent_path().string() + '/' + out_gltf_model_name + ".gltf"});
            }
        }
    }

    std::ofstream new_objects_ref(combine_path(out_gltf_route_path, "objects.ref"), std::ios::out);
    if (!new_objects_ref.is_open())
    {
        std::cerr << "Failed to create new objects.ref" << std::endl;
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
        std::cerr << "Failed to open " << in_texture_path << std::endl;
        return false;
    }

    std::string texture_ext = fs::path(in_texture_path).extension().string();
    model_data.is_TGA_texture = texture_ext == ".tga";

    auto last_slash_pos = out_gltf_model_path.find_last_of(separator());

    std::string gltf_directory_path = "";

    if (last_slash_pos == std::string::npos)
    {
        gltf_directory_path = ".";
    }
    else
    {
        gltf_directory_path = out_gltf_model_path.substr(0, last_slash_pos);
    }


    if (!get_dmd_model_data(in_dmd_model_path, model_data))
    {
        return false;
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
        std::cerr << "Failed to open " << in_dmd_model_path << std::endl;
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
            std::cerr << "Failed to find \"TriMesh()\" in " << in_dmd_model_path << std::endl;
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
        std::cerr << "Failed to read positions from " << in_dmd_model_path << std::endl;
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
        std::cerr << "Failed to read position indices from " << in_dmd_model_path << std::endl;
        return false;
    }

    while (buffer != "Texture:")
    {
        model_file >> buffer;
        if (!model_file)
        {
            std::cerr << "Failed to find \"Texture:\" in " << in_dmd_model_path << std::endl;
            return false;
        }
    }

    model_file >> buffer >> buffer;

    std::uint32_t tex_coord_count, tex_face_count;
    model_file >> tex_coord_count >> tex_face_count;

    if (pos_face_count != tex_face_count)
    {
        std::cerr << "Position face count is not equal to texture face count in " << in_dmd_model_path << std::endl;
        return false;
    }

    std::uint32_t face_count = pos_face_count;

    model_file >> buffer >> buffer;

    std::vector<Vec2> tex_coords(tex_coord_count);
    for (auto& tex_coord : tex_coords)
    {
        model_file >> tex_coord.x >> tex_coord.y >> buffer;

        if (!model_data.is_TGA_texture)
        {
            tex_coord.y = 1.0f - tex_coord.y;
        }
    }

    if (!model_file)
    {
        std::cerr << "Failed to read texture coordinates from " << in_dmd_model_path << std::endl;
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
        std::cerr << "Failed to read texture indices from " << in_dmd_model_path << std::endl;
        return false;
    }

    model_file.close();

    model_data.vertices.reserve(face_count * 3);
    model_data.indices.resize(face_count * 3);

    std::map<std::pair<PosIndex, TexIndex>, VertexIndex> unique_indices;

    for (std::uint32_t i = 0; i < face_count * 3; ++i)
    {
        PosIndex pos_index = pos_indices[i];
        TexIndex tex_index = tex_indices[i];

        auto found_it = unique_indices.find({pos_index, tex_index});
        if (found_it == unique_indices.end())
        {
            model_data.vertices.emplace_back(Vertex{positions[pos_index], tex_coords[tex_index]});

            VertexIndex new_vertex_index = model_data.vertices.size() - 1;
            model_data.indices[i] = new_vertex_index;
            unique_indices.insert({{pos_index, tex_index}, new_vertex_index});
        }
        else
        {
            model_data.indices[i] = found_it->second;
        }
    }

    model_data.vertices.shrink_to_fit();

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::generate_gltf_model(Geometry& model_data,
                                      std::string &in_texture_path,
                                      std::string &gltf_directory_path,
                                      std::string &out_relative_bin_path,
                                      std::string &out_relative_texture_path)
{
    for (auto& vertex : model_data.vertices)
    {
        std::swap(vertex.pos.y, vertex.pos.z);
        vertex.pos.z = -vertex.pos.z;
    }

    std::string full_bin_path = combine_path(gltf_directory_path, out_relative_bin_path);

    std::ofstream bin_file(full_bin_path, std::ios::binary | std::ios::out);
    if (!bin_file.is_open())
    {
        std::cerr << "Failed to open " << full_bin_path << std::endl;
        return false;
    }

    for (const auto& vertex : model_data.vertices)
    {
        bin_file.write(reinterpret_cast<const char*>(&vertex.pos), sizeof(vertex.pos));
    }

    auto positions_byte_length = bin_file.tellp();

    for (const auto& vertex : model_data.vertices)
    {
        bin_file.write(reinterpret_cast<const char*>(&vertex.tex_coord), sizeof(vertex.tex_coord));
    }

    auto tex_coords_byte_length = bin_file.tellp() - positions_byte_length;

    for (auto index : model_data.indices)
    {
        bin_file.write(reinterpret_cast<const char*>(&index), sizeof(index));
    }

    auto indices_byte_length = bin_file.tellp() - tex_coords_byte_length - positions_byte_length;

    bin_file.close();

    Vec3 min_pos, max_pos;
    Vec2 min_tex, max_tex;
    min_pos = max_pos = model_data.vertices[0].pos;
    min_tex = max_tex = model_data.vertices[0].tex_coord;

    for (const auto& vertex : model_data.vertices)
    {
        min_pos.x = std::min(min_pos.x, vertex.pos.x);
        min_pos.y = std::min(min_pos.y, vertex.pos.y);
        min_pos.z = std::min(min_pos.z, vertex.pos.z);

        max_pos.x = std::max(max_pos.x, vertex.pos.x);
        max_pos.y = std::max(max_pos.y, vertex.pos.y);
        max_pos.z = std::max(max_pos.z, vertex.pos.z);

        min_tex.x = std::min(min_tex.x, vertex.tex_coord.x);
        min_tex.y = std::min(min_tex.y, vertex.tex_coord.y);

        max_tex.x = std::max(max_tex.x, vertex.tex_coord.x);
        max_tex.y = std::max(max_tex.y, vertex.tex_coord.y);
    }

    std::string gltf_path = combine_path(gltf_directory_path, model_data.model_file_name + ".gltf");

    std::ofstream gltf_file(gltf_path, std::ios::out);
    if (!gltf_file.is_open())
    {
        std::cerr << "Failed to open " << gltf_path << std::endl;
        return false;
    }

    gltf_file << "{\n"
        "    \"asset\": {\n"
        "        \"version\": \"2.0\"\n"
        "    },\n"
        "    \"buffers\": [\n"
        "        {\n"
        "            \"uri\": \"" << out_relative_bin_path << "\",\n"
        "            \"byteLength\": " << positions_byte_length + tex_coords_byte_length + indices_byte_length << "\n"
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
        "            \"byteLength\": " << tex_coords_byte_length << ",\n"
        "            \"target\": 34962\n"
        "        },\n"
        "        {\n"
        "            \"buffer\": 0,\n"
        "            \"byteOffset\": " << positions_byte_length + tex_coords_byte_length << ",\n"
        "            \"byteLength\": " << indices_byte_length << ",\n"
        "            \"target\": 34963\n"
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
        "            \"type\": \"VEC2\",\n"
        "            \"max\": [\n"
        "                " << max_tex.x << ",\n"
        "                " << max_tex.y << "\n"
        "            ],\n"
        "            \"min\": [\n"
        "                " << min_tex.x << ",\n"
        "                " << min_tex.y << "\n"
        "            ]\n"
        "        },\n"
        "        {\n"
        "            \"bufferView\": 2,\n"
        "            \"componentType\": 5125,\n"
        "            \"count\": " << model_data.indices.size() << ",\n"
        "            \"type\": \"SCALAR\",\n"
        "            \"max\": [\n"
        "                " << model_data.vertices.size() - 1 << "\n"
        "            ],\n"
        "            \"min\": [\n"
        "                0\n"
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
        "            },\n"
        "            \"alphaMode\": \"BLEND\"\n"
        "        }\n"
        "    ],\n"
        "    \"meshes\": [\n"
        "        {\n"
        "            \"primitives\": [\n"
        "                {\n"
        "                    \"attributes\": {\n"
        "                        \"POSITION\": 0,\n"
        "                        \"TEXCOORD_0\": 1\n"
        "                    },\n"
        "                    \"indices\": 2,\n"
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
            std::cerr << e.what() << std::endl;
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

    parser.set_optional<bool>("u", "only-used",
                              false,
                              "Convert only models used at map");

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
            std::cerr << "ERROR: Missing route output path" << std::endl;
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
                std::cerr << "ERROR: Missing output GLTF model path" << std::endl;
            }
        }
        else
        {
            std::cerr << "ERROR: Missing input DMD texture path" << std::endl;
        }
    }
    else
    {
        std::cerr << "ERROR: Missing input route path or DMD model path" << std::endl;
    }

    return false;
}
