#include "Route.h"

#include "EditorContext.h"
#include "PagedLodMap.h"
#include "RouteMap.h"
#include "RouteObject.h"
#include "Settings.h"
#include "filesystem.h"
#include "rail-signal.h"
#include "signals-data-types.h"
#include "topology.h"
#include "vec3.h"

#include <CfgReader.h>

#include <vsg/app/RecordTraversal.h>
#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/common.h>
#include <vsg/maths/sphere.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/InstanceNode.h>

#include <QString>

// #include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <filesystem>
// #include <fstream>
// #include <sstream>
#include <string>
#include <utility>
// #include <vector>

#define LABEL_BUFFER_SIZE 256
#define RELATIVE_PATH_BUFFER_SIZE 512
#define FLOAT_BUFFER_SIZE 32

static vsg::vec3 to_vsg_vec3(dvec3 vec)
{
    return vsg::vec3{
        static_cast<float>(vec.x),
        static_cast<float>(vec.y),
        static_cast<float>(vec.z)
    };
}

static bool to_float(const char* buf, float& out)
{
    char* endptr;
    errno = 0;
    const float result = std::strtof(buf, &endptr);
    if (errno == 0 && *endptr == '\0')
    {
        out = result;
        return true;
    }
    else
    {
        return false;
    }
}

static char* read_file_in_buffer(const char* filename, const char* modes)
{
    std::FILE* const file = std::fopen(filename, modes);
    if (!file)
    {
        // TODO: Replace on Journal
        std::fprintf(stderr, "Failed to open %s\n", filename);
        return nullptr;
    }

    std::fseek(file, 0, SEEK_END);
    const long buffer_length = std::ftell(file);
    std::rewind(file);

    char* const buffer = reinterpret_cast<char*>(
        std::malloc(buffer_length + 1));

    if (!buffer)
    {
        // TODO: Replace on Journal
        std::fprintf(stderr, "Failed to allocate memory for %s content\n",
            filename);

        std::fclose(file);
        return nullptr;
    }

    const std::size_t bytes_read = std::fread(buffer, 1, buffer_length, file);
    buffer[buffer_length] = '\0';

    std::fclose(file);

    if (bytes_read < static_cast<std::size_t>(buffer_length))
    {
        // TODO: Replace on Journal
        std::fprintf(stderr, "Failed to read %s\n", filename);
        std::free(buffer);
        return nullptr;
    }

    return buffer;
}

Route::Route(EditorContext& context)
    : context(context)
{
    const bool success = load_objects_ref() && load_route_map();
    if (!success)
    {
        return;
    }

    const FileSystem& fs = FileSystem::getInstance();

    PagedLodMap paged_lods;
    for (const auto& [label, relative_path] : context.objects_ref)
    {
        const auto paged_lod = vsg::PagedLOD::create();
        paged_lod->filename = fs.combinePath(context.route_dir, relative_path);

        paged_lod->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0),
            static_cast<double>(context.settings.view_distance));

        paged_lod->children.front() = {0.1, nullptr};
        paged_lod->options = context.options;

        paged_lods.emplace(label, std::move(paged_lod));
    }

    load_static_objects(paged_lods);
    load_topology();
}

bool Route::load_objects_ref()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string objects_ref_path = fs.combinePath(
        context.route_dir, "objects.ref");

    char* const buffer = read_file_in_buffer(objects_ref_path.c_str(), "r");
    if (!buffer)
    {
        return false;
    }

    enum State
    {
        INITIAL,
        START_LABEL,
        FINISH_LABEL,
        START_RELATIVE_PATH,
        FINISH_RELATIVE_PATH
    };

    char label[LABEL_BUFFER_SIZE];
    std::uint8_t label_length = 0;
    char relative_path[RELATIVE_PATH_BUFFER_SIZE];
    std::uint16_t relative_path_length = 0;
    State state = INITIAL;

    for (const char* ptr = buffer; *ptr != '\0'; ++ptr)
    {
        switch (*ptr)
        {
            case '\n':
            {
                if (state >= START_RELATIVE_PATH)
                {
                    relative_path[relative_path_length] = '\0';
                    context.objects_ref.emplace(label, relative_path);
                }

                label_length = 0;
                relative_path_length = 0;
                state = INITIAL;

                break;
            }
            case ' ': case '\t': case '\r':
            {
                switch (state)
                {
                    case START_LABEL:
                    {
                        label[label_length] = '\0';
                        state = FINISH_LABEL;
                        break;
                    }
                    case START_RELATIVE_PATH:
                    {
                        relative_path[relative_path_length] = '\0';
                        state = FINISH_RELATIVE_PATH;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }

                break;
            }
            default:
            {
                switch (state)
                {
                    case INITIAL:
                    {
                        label[label_length] = *ptr;
                        ++label_length;
                        state = START_LABEL;
                        break;
                    }
                    case START_LABEL:
                    {
                        if (label_length == LABEL_BUFFER_SIZE - 1)
                        {
                            label[label_length] = '\0';

                            // TODO: Replace on Journal
                            std::fprintf(stderr, "Objects ref: Could not append"
                                "to label buffer %s\n", label);

                            break;
                        }

                        label[label_length] = *ptr;
                        ++label_length;
                        break;
                    }
                    case FINISH_LABEL:
                    {
                        relative_path[relative_path_length] = *ptr;
                        ++relative_path_length;
                        state = START_RELATIVE_PATH;
                        break;
                    }
                    case START_RELATIVE_PATH:
                    {
                        if (relative_path_length ==
                            RELATIVE_PATH_BUFFER_SIZE - 1)
                        {
                            relative_path[relative_path_length] = '\0';

                            // TODO: Replace on Journal
                            std::fprintf(stderr, "Objects ref: Could not append"
                                "to relative path buffer %s\n", relative_path);

                            break;
                        }

                        relative_path[relative_path_length] = *ptr;
                        ++relative_path_length;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }
    }

    std::free(buffer);

    // std::ifstream objects_ref_file(objects_ref_path);
    // if (!objects_ref_file)
    // {
    //     // TODO: Replace on Journal
    //     std::fprintf(stderr, "Failed to open %s\n", objects_ref_path.c_str());
    //     return false;
    // }

    // std::string line;
    // while (std::getline(objects_ref_file, line))
    // {
    //     std::istringstream iss(std::move(line));
    //     std::string label, relative_path;

    //     if (iss >> label >> relative_path)
    //     {
    //         context.objects_ref.emplace(std::move(label),
    //             std::move(relative_path));
    //     }
    // }

    return true;
}

bool Route::load_route_map()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string route_map_path = fs.combinePath(context.route_dir,
        "topology", "map", "route1.map");

    char* const buffer = read_file_in_buffer(route_map_path.c_str(), "r");
    if (!buffer)
    {
        return false;
    }

    enum State
    {
        INITIAL,
        START_LABEL,
        FINISH_LABEL,
        START_TRANSLATION_X,
        FINISH_TRANSLATION_X,
        START_TRANSLATION_Y,
        FINISH_TRANSLATION_Y,
        START_TRANSLATION_Z,
        FINISH_TRANSLATION_Z,
        START_ROTATION_X,
        FINISH_ROTATION_X,
        START_ROTATION_Y,
        FINISH_ROTATION_Y,
        START_ROTATION_Z,
        FINISH_ROTATION_Z
    };

    char label[LABEL_BUFFER_SIZE];
    std::uint8_t label_length = 0;
    char float_buffer[FLOAT_BUFFER_SIZE];
    std::uint8_t float_buffer_length = 0;
    vsg::vec3 translation;
    vsg::vec3 rotation;
    State state = INITIAL;

    for (const char* ptr = buffer; *ptr != '\0'; ++ptr)
    {
        switch (*ptr)
        {
            case '\n':
            {
                if (state >= START_ROTATION_Z)
                {
                    if (state == START_ROTATION_Z)
                    {
                        float_buffer[float_buffer_length] = '\0';

                        if (!to_float(float_buffer, rotation.z))
                        {
                            // TODO: Replace on Journal
                            std::fprintf(stderr, "Route map: Wrong float value "
                                "%s\n", float_buffer);
                        }
                    }

                    context.route_map[label].emplace_back(
                        RouteMapTransformation{translation, rotation});
                }

                label_length = 0;
                float_buffer_length = 0;
                state = INITIAL;

                break;
            }
            case ' ': case '\t': case ',': case ';': case '\r':
            {
                switch (state)
                {
                    case START_LABEL:
                    {
                        label[label_length] = '\0';
                        state = FINISH_LABEL;
                        break;
                    }
                    case START_TRANSLATION_X:
                    {
                        float_buffer[float_buffer_length] = '\0';

                        if (!to_float(float_buffer, translation.x))
                        {
                            // TODO: Replace on Journal
                            std::fprintf(stderr, "Route map: Wrong float value "
                                "%s\n", float_buffer);
                        }

                        float_buffer_length = 0;
                        state = FINISH_TRANSLATION_X;

                        break;
                    }
                    case START_TRANSLATION_Y:
                    {
                        float_buffer[float_buffer_length] = '\0';

                        if (!to_float(float_buffer, translation.y))
                        {
                            // TODO: Replace on Journal
                            std::fprintf(stderr, "Route map: Wrong float value "
                                "%s\n", float_buffer);
                        }

                        float_buffer_length = 0;
                        state = FINISH_TRANSLATION_Y;

                        break;
                    }
                    case START_TRANSLATION_Z:
                    {
                        float_buffer[float_buffer_length] = '\0';

                        if (!to_float(float_buffer, translation.z))
                        {
                            // TODO: Replace on Journal
                            std::fprintf(stderr, "Route map: Wrong float value "
                                "%s\n", float_buffer);
                        }

                        float_buffer_length = 0;
                        state = FINISH_TRANSLATION_Z;

                        break;
                    }
                    case START_ROTATION_X:
                    {
                        float_buffer[float_buffer_length] = '\0';

                        if (!to_float(float_buffer, rotation.x))
                        {
                            // TODO: Replace on Journal
                            std::fprintf(stderr, "Route map: Wrong float value "
                                "%s\n", float_buffer);
                        }

                        float_buffer_length = 0;
                        state = FINISH_ROTATION_X;

                        break;
                    }
                    case START_ROTATION_Y:
                    {
                        float_buffer[float_buffer_length] = '\0';

                        if (!to_float(float_buffer, rotation.y))
                        {
                            // TODO: Replace on Journal
                            std::fprintf(stderr, "Route map: Wrong float value "
                                "%s\n", float_buffer);
                        }

                        float_buffer_length = 0;
                        state = FINISH_ROTATION_Y;

                        break;
                    }
                    case START_ROTATION_Z:
                    {
                        float_buffer[float_buffer_length] = '\0';

                        if (!to_float(float_buffer, rotation.z))
                        {
                            // TODO: Replace on Journal
                            std::fprintf(stderr, "Route map: Wrong float value "
                                "%s\n", float_buffer);
                        }

                        float_buffer_length = 0;
                        state = FINISH_ROTATION_Z;

                        break;
                    }
                    default:
                    {
                        break;
                    }
                }

                break;
            }
            default:
            {
                switch (state)
                {
                    case INITIAL:
                    {
                        label[label_length] = *ptr;
                        ++label_length;
                        state = START_LABEL;
                        break;
                    }
                    case START_LABEL:
                    {
                        if (label_length == LABEL_BUFFER_SIZE - 1)
                        {
                            label[label_length] = '\0';

                            // TODO: Replace on Journal
                            std::fprintf(stderr, "Objects ref: Could not append"
                                "to label buffer %s\n", label);

                            break;
                        }

                        label[label_length] = *ptr;
                        ++label_length;
                        break;
                    }
                    case FINISH_LABEL:
                    {
                        float_buffer[float_buffer_length] = *ptr;
                        ++float_buffer_length;
                        state = START_TRANSLATION_X;
                        break;
                    }
                    case START_TRANSLATION_X: case START_TRANSLATION_Y:
                    case START_TRANSLATION_Z: case START_ROTATION_X:
                    case START_ROTATION_Y: case START_ROTATION_Z:
                    {
                        if (float_buffer_length == FLOAT_BUFFER_SIZE - 1)
                        {
                            float_buffer[float_buffer_length] = '\0';

                            // TODO: Replace on Journal
                            std::fprintf(stderr, "Route map: Could not append"
                                "to float buffer %s\n", float_buffer);

                            break;
                        }

                        float_buffer[float_buffer_length] = *ptr;
                        ++float_buffer_length;
                        break;
                    }
                    case FINISH_TRANSLATION_X:
                    {
                        float_buffer[float_buffer_length] = *ptr;
                        ++float_buffer_length;
                        state = START_TRANSLATION_Y;
                        break;
                    }
                    case FINISH_TRANSLATION_Y:
                    {
                        float_buffer[float_buffer_length] = *ptr;
                        ++float_buffer_length;
                        state = START_TRANSLATION_Z;
                        break;
                    }
                    case FINISH_TRANSLATION_Z:
                    {
                        float_buffer[float_buffer_length] = *ptr;
                        ++float_buffer_length;
                        state = START_ROTATION_X;
                        break;
                    }
                    case FINISH_ROTATION_X:
                    {
                        float_buffer[float_buffer_length] = *ptr;
                        ++float_buffer_length;
                        state = START_ROTATION_Y;
                        break;
                    }
                    case FINISH_ROTATION_Y:
                    {
                        float_buffer[float_buffer_length] = *ptr;
                        ++float_buffer_length;
                        state = START_ROTATION_Z;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }

                break;
            }
        }
    }

    std::free(buffer);

    // std::ifstream route_map_file(route_map_path);
    // if (!route_map_file)
    // {
    //     // TODO: Replace on Journal
    //     std::fprintf(stderr, "Failed to open %s\n", route_map_path.c_str());
    //     return false;
    // }

    // std::string line;
    // while (std::getline(route_map_file, line))
    // {
    //     if (line.empty())
    //     {
    //         continue;
    //     }

    //     if (line.back() == ';')
    //     {
    //         line.pop_back();
    //     }

    //     std::replace(line.begin(), line.end(), ',', ' ');

    //     std::istringstream iss(std::move(line));
    //     std::string label;
    //     vsg::vec3 translation, rotation;

    //     if (iss >> label >> translation >> rotation)
    //     {
    //         context.route_map[label].emplace_back(
    //             RouteMapTransformation{translation, rotation});
    //     }
    // }

    return true;
}

void Route::load_static_objects(const PagedLodMap& paged_lods)
{
    for (const auto& [label, transforms] : context.route_map)
    {
        const auto paged_lod_it = paged_lods.find(label);
        if (paged_lod_it == paged_lods.cend())
        {
            continue;
        }

        for (const auto& transform : transforms)
        {
            const auto object = RouteObject::create(context, paged_lod_it->second,
                label, transform.translation, -transform.rotation_deg);

            this->addChild(vsg::MASK_ALL, object);
        }
    }
}

bool Route::load_topology()
{
    const FileSystem& fs = FileSystem::getInstance();

    const auto cfg_path = fs.combinePath(context.route_dir,
        "topology", "models-config.xml");

    CfgReader cfg;
    if (!cfg.load(QString::fromStdString(cfg_path)))
    {
        // TODO: Replace on Journal
        std::fprintf(stderr, "Failed to load %s\n", cfg_path.c_str());

        return false;
    }

    const QString section_name = "Models";
    QString signal_models_dir;

    if (!cfg.getString(section_name, "SignalModelsDir", signal_models_dir))
    {
        // TODO: Replace on Journal
        std::fprintf(stderr, "Failed to find field \"SignalModelsDir\" "
            "in section %s in %s\n", section_name.toStdString().c_str(),
            cfg_path.c_str());

        return false;
    }

    const std::string models_dir_name = signal_models_dir.toStdString();

    const std::string models_dir = fs.combinePath(fs.getDataDir(),
        "models", models_dir_name);

    // TODO: Replace on Journal
    std::printf("Signals directory: %s\n", models_dir.c_str());

    context.topology = new Topology;

    const auto directory_filename = std::filesystem::path(context.route_dir).filename();

    if (!context.topology->load(directory_filename.string().c_str()))
    {
        // TODO: Replace on Journal
        std::fputs("Failed to load topology\n", stderr);

        return false;
    }

    PagedLodMap paged_lods;

    const signals_data_t* const signals_data = context.topology->getSignalsData();
    if (!signals_data)
    {
        return false;
    }

    const auto load_signal = [&](Signal* const signal) -> void
    {
        if (!signal)
        {
            // TODO: Replace on Journal
            std::fprintf(stderr, "Invalid signal %p\n", (void*)signal);
            return;
        }

        const std::string signal_model_name =
            signal->getSignalModel().toStdString();

        if (signal_model_name.empty() || signal_model_name == "empty_line")
        {
            return;
        }

        const std::string signal_model_path = fs.combinePath(
            models_dir, signal_model_name) + ".gltf";

        vsg::ref_ptr<vsg::PagedLOD> paged_lod;

        auto paged_lod_it = paged_lods.find(signal_model_path);
        if (paged_lod_it == paged_lods.end())
        {
            const auto new_paged_lod = vsg::PagedLOD::create();
            new_paged_lod->filename = signal_model_path;

            new_paged_lod->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0),
                static_cast<double>(context.settings.view_distance));

            new_paged_lod->children.front() = {0.1, nullptr};
            new_paged_lod->options = context.options;

            paged_lod_it = paged_lods.emplace(signal_model_path,
                new_paged_lod).first;
        }

        paged_lod = paged_lod_it->second;

        signal->calcPosition();

        const auto pos = to_vsg_vec3(signal->getPos());
        const auto right = to_vsg_vec3(signal->getRight());
        const auto orth = to_vsg_vec3(signal->getOrth());
        const auto up = to_vsg_vec3(signal->getUp());

        const vsg::vec3 rotation_deg = {
            vsg::degrees(std::atan2(orth.z, up.z)),
            vsg::degrees(std::atan2(-right.z, std::hypot(orth.z, up.z))),
            vsg::degrees(std::atan2(-right.y, right.x))
        };

        const auto object = RouteObject::create(context, paged_lod,
            signal_model_name, pos, rotation_deg);

        this->addChild(vsg::MASK_ALL, object);
    };

    for (Signal* const line_signal : signals_data->line_signals)
    {
        load_signal(line_signal);
    }

    for (Signal* const enter_signal : signals_data->enter_signals)
    {
        load_signal(enter_signal);
    }

    for (Signal* const exit_signal : signals_data->exit_signals)
    {
        load_signal(exit_signal);
    }

    return true;
}
