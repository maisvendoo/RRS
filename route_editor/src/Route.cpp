#include "Route.h"

#include "EditorContext.h"
#include "PagedLodMap.h"
#include "RouteMap.h"
#include "RouteObject.h"
#include "Settings.h"
#include "filesystem.h"
// #include "parse_file_funcs.h"
#include "rail-signal.h"
#include "signals-data-types.h"
#include "topology.h"
#include "vec3.h"

#include <CfgReader.h>

#include <fstream>
#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/common.h>
#include <vsg/maths/sphere.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/PagedLOD.h>

#include <QString>

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <string>

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

Route::Route(EditorContext& context)
    : context(context)
{
    const bool success = load_objects_ref() && load_route_map();
    if (!success)
    {
        return;
    }

    const FileSystem& fs = FileSystem::getInstance();

    for (auto& [label, ref] : context.objects_ref)
    {
        const auto paged_lod = vsg::PagedLOD::create();
        paged_lod->filename = fs.combinePath(context.route_dir,
            ref.relative_path);

        paged_lod->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0),
            static_cast<double>(context.settings.view_distance));

        paged_lod->children.front() = {0.1, nullptr};
        paged_lod->options = context.options;

        ref.paged_lod = paged_lod;
    }

    load_static_objects();
    load_topology();
}

bool Route::load_objects_ref()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string objects_ref_path = fs.combinePath(
        context.route_dir, "objects.ref");

    // char label[LABEL_BUFFER_SIZE];
    // char relative_path[RELATIVE_PATH_BUFFER_SIZE];

    // return parse_file_line_by_line(
    //     objects_ref_path.c_str(), "r", " \t\r",
    //     [&]() -> void {
    //         context.objects_ref.emplace(label, relative_path);
    //     },
    //     {
    //         ParseField::String(label, LABEL_BUFFER_SIZE),
    //         ParseField::String(relative_path, RELATIVE_PATH_BUFFER_SIZE)
    //     }
    // );

    std::ifstream objects_ref_file(objects_ref_path);
    if (!objects_ref_file)
    {
        // TODO: Replace on Journal
        std::fprintf(stderr, "Failed to open %s\n", objects_ref_path.c_str());
        return false;
    }

    std::string line;
    while (std::getline(objects_ref_file, line))
    {
        std::istringstream iss(std::move(line));
        std::string label, relative_path;

        if (iss >> label >> relative_path)
        {
            context.objects_ref.emplace(std::move(label),
                ObjectRef{std::move(relative_path), nullptr});
        }
    }

    return true;
}

bool Route::load_route_map()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string route_map_path = fs.combinePath(context.route_dir,
        "topology", "map", "route1.map");

    // char label[LABEL_BUFFER_SIZE];
    // char float_buffer[FLOAT_BUFFER_SIZE];
    // vsg::vec3 translation;
    // vsg::vec3 rotation_deg;

    // return parse_file_line_by_line(
    //     route_map_path.c_str(), "r", " \t\r,;",
    //     [&]() -> void {
    //         context.route_map[label].emplace_back(
    //             RouteMapTransformation{translation, rotation_deg});
    //     },
    //     {
    //         ParseField::String(label, LABEL_BUFFER_SIZE),
    //         ParseField::Float(float_buffer, FLOAT_BUFFER_SIZE, &translation.x),
    //         ParseField::Float(float_buffer, FLOAT_BUFFER_SIZE, &translation.y),
    //         ParseField::Float(float_buffer, FLOAT_BUFFER_SIZE, &translation.z),
    //         ParseField::Float(float_buffer, FLOAT_BUFFER_SIZE, &rotation_deg.x),
    //         ParseField::Float(float_buffer, FLOAT_BUFFER_SIZE, &rotation_deg.y),
    //         ParseField::Float(float_buffer, FLOAT_BUFFER_SIZE, &rotation_deg.z)
    //     }
    // );

    std::ifstream route_map_file(route_map_path);
    if (!route_map_file)
    {
        // TODO: Replace on Journal
        std::fprintf(stderr, "Failed to open %s\n", route_map_path.c_str());
        return false;
    }

    std::string line;
    while (std::getline(route_map_file, line))
    {
        if (line.empty())
        {
            continue;
        }

        if (line.back() == ';')
        {
            line.pop_back();
        }

        std::replace(line.begin(), line.end(), ',', ' ');

        std::istringstream iss(std::move(line));
        std::string label;
        vsg::vec3 translation, rotation;

        if (iss >> label >> translation >> rotation)
        {
            context.route_map[label].emplace_back(
                RouteMapTransformation{translation, rotation});
        }
    }

    return true;
}

void Route::load_static_objects()
{
    for (const auto& [label, transforms] : context.route_map)
    {
        const auto ref_it = context.objects_ref.find(label);
        if (ref_it == context.objects_ref.cend())
        {
            continue;
        }

        for (const auto& transform : transforms)
        {
            const auto object = RouteObject::create(context,
                ref_it->second.paged_lod, label, transform.translation,
                -transform.rotation_deg);

            this->addChild(vsg::MASK_ALL, object);

            context.static_objects.emplace_back(object);
        }
    }
}

bool Route::load_topology()
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string cfg_path = fs.combinePath(context.route_dir,
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

    const auto directory_name = std::filesystem::path(
        context.route_dir).filename();

    if (!context.topology->load(directory_name.string().c_str()))
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

        const vsg::vec3 pos = to_vsg_vec3(signal->getPos());
        const vsg::vec3 right = to_vsg_vec3(signal->getRight());
        const vsg::vec3 orth = to_vsg_vec3(signal->getOrth());
        const vsg::vec3 up = to_vsg_vec3(signal->getUp());

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
