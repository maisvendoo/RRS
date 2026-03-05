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

#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/common.h>
#include <vsg/maths/sphere.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/PagedLOD.h>

#include <QString>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

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
    const auto objects_ref_path = fs.combinePath(context.route_dir, "objects.ref");

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
            context.objects_ref.emplace(std::move(label), std::move(relative_path));
        }
    }

    return true;
}

bool Route::load_route_map()
{
    const FileSystem& fs = FileSystem::getInstance();

    const auto route_map_path = fs.combinePath(context.route_dir,
        "topology", "map", "route1.map");

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
            context.route_map.emplace(std::move(label),
                RouteMapTransformation{translation, rotation});
        }
    }

    return true;
}

void Route::load_static_objects(const PagedLodMap& paged_lods)
{
    for (const auto& [label, transform] : context.route_map)
    {
        const auto paged_lod_it = paged_lods.find(label);
        if (paged_lod_it == paged_lods.cend())
        {
            continue;
        }

        const auto object = RouteObject::create(context, paged_lod_it->second,
            label, transform.translation, -transform.rotation);

        this->addChild(vsg::MASK_ALL, object);
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

    const auto directory_stem = std::filesystem::path(context.route_dir).stem();

    if (!context.topology->load(directory_stem.string().c_str()))
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

    const auto load_signals = [&](const std::vector<Signal*>& signals_) -> void
    {
        for (Signal* const signal : signals_)
        {
            if (!signal)
            {
                // TODO: Replace on Journal
                std::fprintf(stderr, "Invalid signal %p\n", (void*)signal);

                continue;
            }

            const std::string signal_model_name =
                signal->getSignalModel().toStdString();

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
                    std::move(new_paged_lod)).first;
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
        }
    };

    load_signals(signals_data->line_signals);
    load_signals(signals_data->enter_signals);
    load_signals(signals_data->exit_signals);

    return true;
}
