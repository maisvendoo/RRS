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
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <filesystem>
#include <functional>
// #include <fstream>
// #include <sstream>
#include <string>
#include <utility>
// #include <vector>

#define LABEL_BUFFER_SIZE 256
#define RELATIVE_PATH_BUFFER_SIZE 512
#define FLOAT_BUFFER_SIZE 32
#define LINE_BUFFER_SIZE 1024

static vsg::vec3 to_vsg_vec3(dvec3 vec)
{
    return vsg::vec3{
        static_cast<float>(vec.x),
        static_cast<float>(vec.y),
        static_cast<float>(vec.z)
    };
}

static bool to_float(const char* buf, float* out)
{
    char* endptr;
    errno = 0;
    const float result = std::strtof(buf, &endptr);
    if (errno == 0 && *endptr == '\0')
    {
        *out = result;
        return true;
    }
    else
    {
        return false;
    }
}

static bool to_double(const char* buf, double* out)
{
    char* endptr;
    errno = 0;
    const double result = std::strtod(buf, &endptr);
    if (errno == 0 && *endptr == '\0')
    {
        *out = result;
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

enum Type
{
    TYPE_STRING,
    TYPE_FLOAT,
    TYPE_DOUBLE
};

struct Field
{
    Type type;
    char* buffer;
    std::size_t buffer_length;
    std::size_t buffer_size;
    void* out;

    bool process(const char* filename, int line_num, char* line_buffer)
    {
        buffer[buffer_length] = '\0';
        buffer_length = 0;

        switch (type)
        {
            case TYPE_FLOAT:
            {
                if (!to_float(buffer, reinterpret_cast<float*>(out)))
                {
                    std::fprintf(stderr, "%s:%d: error: failed to "
                        "read float value\n    %s\n",
                        filename, line_num, line_buffer);

                    return false;
                }

                return true;
            }
            case TYPE_DOUBLE:
            {
                if (!to_double(buffer, reinterpret_cast<double*>(out)))
                {
                    std::fprintf(stderr, "%s:%d: error: failed to "
                        "read double value\n    %s\n",
                        filename, line_num, line_buffer);

                    return false;
                }

                return true;
            }
            default:
            {
                return true;
            }
        }
    }

    bool append_char(const char* filename, int line_num,
        char* line_buffer, char ch)
    {
        buffer[buffer_length] = ch;
        ++buffer_length;

        if (buffer_length == buffer_size)
        {
            std::fprintf(stderr, "%s:%d: error: value is not fitting into "
                "buffer\n    %s\n", filename, line_num, line_buffer);

            return false;
        }

        return true;
    }
};

static bool read_line_by_line(const char* filename, const char* modes,
    const char* separators, std::function<void()> function, int argc, ...)
{
    char* const buffer = read_file_in_buffer(filename, modes);
    if (!buffer)
    {
        return false;
    }

    Field* const fields = reinterpret_cast<Field*>(
        std::malloc(sizeof(Field) * argc));

    if (!fields)
    {
        std::fprintf(stderr, "Failed to allocate memory for %s fields\n",
            filename);

        std::free(buffer);
        return false;
    }

    std::va_list args;
    va_start(args, argc);

    for (int i = 0; i < argc; ++i)
    {
        Field* const field = &fields[i];
        field->type = va_arg(args, Type);
        field->buffer = va_arg(args, char*);
        field->buffer_length = 0;
        field->buffer_size = va_arg(args, std::size_t);

        if (field->type != TYPE_STRING)
        {
            field->out = va_arg(args, void*);
        }
    }

    va_end(args);

    // Допусти у нас 5(N) аргументов, тогда:
    // state 0 - до чтения первого элемента
    // state 1 - во время чтения первого элемента
    // state 2 - до чтения второго элемента
    // ...
    // state 9(2N - 1) - во время чтения пятого элемента
    // state 2N - прочитали все, что должно быть в строчке

    // Если state == 0, то пустая строка - игнорируем;
    // Если state == 2N - 1, то последний элемент еще нужно записать;
    // Если state == 2N, то все нормально;
    // Иначе данные в строке неверные - выходим с ошибкой

    int curr_state = 0;
    const char* ptr = buffer;
    int line_num = 1;
    char line_buffer[LINE_BUFFER_SIZE];
    int line_length = 0;

/*
0 - обновить линии, но ничего не делать
2N - 1 - обработать значение, вызвать function, обновить линии
2N - вызвать function, обновить линии
другое - ошибка
*/

    while (true)
    {
        line_buffer[line_length] = *ptr;
        ++line_length;

        if (*ptr == '\0' || *ptr == '\n')
        {
            line_buffer[line_length] = '\0';
            line_length = 0;

            // Newline or EOF right after field - process field
            if (curr_state == 2 * argc - 1)
            {
                if (!fields[(curr_state - 1) / 2].process(
                    filename, line_num, line_buffer))
                {
                    std::free(fields);
                    std::free(buffer);
                    return false;
                }
            }
            else if (curr_state != 0 && curr_state != 2 * argc)
            {
                std::fprintf(stderr, "%s:%d: error: wrong field count\n"
                    "    %s\n", filename, line_num, line_buffer);

                std::free(fields);
                std::free(buffer);
                return false;
            }

            if (curr_state != 0)
            {
                function();
            }

            ++line_num;
            curr_state = 0;

            if (*ptr == '\0')
            {
                break;
            }
        }
        else
        {
            bool is_separator = false;
            for (const char* sep = separators; *sep != '\0'; ++sep)
            {
                if (*ptr == *sep)
                {
                    is_separator = true;
                    break;
                }
            }

            const bool state_is_even = (curr_state % 2 == 0);

            if (is_separator)
            {
                if (!state_is_even)
                {
                    if (!fields[(curr_state - 1) / 2].process(
                        filename, line_num, line_buffer))
                    {
                        std::free(fields);
                        std::free(buffer);
                        return false;
                    }

                    ++curr_state;
                }
            }
            else
            {
                if (curr_state == 2 * argc)
                {
                    std::fprintf(stderr, "%s:%d: error: too many fields in "
                        "line\n    %s\n", filename, line_num, line_buffer);

                    std::free(fields);
                    std::free(buffer);
                    return false;
                }

                if (state_is_even)
                {
                    ++curr_state;
                }

                if (!fields[(curr_state - 1) / 2].append_char(
                    filename, line_num, line_buffer, *ptr))
                {
                    std::free(fields);
                    std::free(buffer);
                    return false;
                }
            }
        }

        ++ptr;
    }

    std::free(fields);
    std::free(buffer);
    return true;
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

    char label[LABEL_BUFFER_SIZE];
    char relative_path[RELATIVE_PATH_BUFFER_SIZE];

    return read_line_by_line(
        objects_ref_path.c_str(), "r", " \t\r",
        [&]() -> void {
            context.objects_ref.emplace(label, relative_path);
        }, 2,
        TYPE_STRING, label, static_cast<std::size_t>(LABEL_BUFFER_SIZE),
        TYPE_STRING, relative_path, static_cast<std::size_t>(RELATIVE_PATH_BUFFER_SIZE)
    );
}

bool Route::load_route_map()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string route_map_path = fs.combinePath(context.route_dir,
        "topology", "map", "route1.map");

    char label[LABEL_BUFFER_SIZE];
    char float_buffer[FLOAT_BUFFER_SIZE];
    vsg::vec3 translation;
    vsg::vec3 rotation;

    return read_line_by_line(
        route_map_path.c_str(), "r", " \t\r,;",
        [&]() -> void {
            context.route_map[label].emplace_back(
                RouteMapTransformation{translation, rotation});
        }, 7,
        TYPE_STRING, label, static_cast<std::size_t>(LABEL_BUFFER_SIZE),
        TYPE_FLOAT, float_buffer, static_cast<std::size_t>(FLOAT_BUFFER_SIZE), &translation.x,
        TYPE_FLOAT, float_buffer, static_cast<std::size_t>(FLOAT_BUFFER_SIZE), &translation.y,
        TYPE_FLOAT, float_buffer, static_cast<std::size_t>(FLOAT_BUFFER_SIZE), &translation.z,
        TYPE_FLOAT, float_buffer, static_cast<std::size_t>(FLOAT_BUFFER_SIZE), &rotation.x,
        TYPE_FLOAT, float_buffer, static_cast<std::size_t>(FLOAT_BUFFER_SIZE), &rotation.y,
        TYPE_FLOAT, float_buffer, static_cast<std::size_t>(FLOAT_BUFFER_SIZE), &rotation.z
    );
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
