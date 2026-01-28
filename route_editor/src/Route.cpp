#include "Route.h"

#include "Mask.h"
#include "ObjectProperties.h"
#include "PagedLodMap.h"
#include "RouteMap.h"
#include "Settings.h"
#include "StringMap.h"
#include "SwitchGroup.h"
#include "filesystem.h"
#include "rail-signal.h"
#include "signals-data-types.h"
#include "topology.h"
#include "vec3.h"

#include <CfgReader.h>

#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/common.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/sphere.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>

#include <QString>

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
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

Route::Route(
    const settings_t& settings,
    vsg::ref_ptr<vsg::Options> options,
    const std::string& directory
)
    : settings(settings)
    , options(options)
    , directory(directory)
{
    assert(options);

    const bool success = load_objects_ref() && load_route_map();
    if (!success)
    {
        return;
    }

    const FileSystem& fs = FileSystem::getInstance();

    PagedLodMap paged_lods;
    for (const auto& [label, relative_path] : objects_ref)
    {
        const auto paged_lod = vsg::PagedLOD::create();
        paged_lod->filename = fs.combinePath(directory, relative_path);

        paged_lod->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0),
            settings.view_distance);

        paged_lod->children.front() = {0.1, nullptr};
        paged_lod->options = options;

        paged_lods[label] = paged_lod;
    }

    load_static_objects(paged_lods);
    load_topology();
}

const StringMap& Route::get_objects_ref() const
{
    return objects_ref;
}

const RouteMap& Route::get_route_map() const
{
    return route_map;
}

const std::unique_ptr<Topology>& Route::get_topology() const
{
    return topology;
}

bool Route::load_objects_ref()
{
    const FileSystem& fs = FileSystem::getInstance();
    const auto objects_ref_path = fs.combinePath(directory, "objects.ref");

    std::ifstream objects_ref_file(objects_ref_path);
    if (!objects_ref_file)
    {
        // TODO: Replace on Journal
        std::fprintf(stderr, "Failed to open %s\n", objects_ref_path.c_str());

        return false;
    }

    std::string line;
    std::istringstream iss;

    while (std::getline(objects_ref_file, line))
    {
        iss.clear();
        iss.str(line);

        std::string label, relative_path;

        if (iss >> label >> relative_path)
        {
            objects_ref[label] = relative_path;
        }
    }

    return true;
}

bool Route::load_route_map()
{
    const FileSystem& fs = FileSystem::getInstance();

    const auto route_map_path = fs.combinePath(directory,
        "topology", "map", "route1.map");

    std::ifstream route_map_file(route_map_path);
    if (!route_map_file)
    {
        // TODO: Replace on Journal
        std::fprintf(stderr, "Failed to open %s\n", route_map_path.c_str());

        return false;
    }

    std::string line;
    std::istringstream iss;

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

        iss.clear();
        iss.str(line);

        std::string label;
        vsg::vec3 translation, rotation;

        if (iss >> label >> translation >> rotation)
        {
            route_map.emplace(label, std::make_pair(translation, rotation));
        }
    }

    return true;
}

void Route::load_static_objects(const PagedLodMap& paged_lods)
{
    for (const auto& [label, transform] : route_map)
    {
        const auto paged_lod_it = paged_lods.find(label);
        if (paged_lod_it == paged_lods.cend())
        {
            continue;
        }

        const vsg::vec3 translation = transform.first;
        const vsg::vec3 rotation_deg = transform.second;

        ObjectProperties properties;
        properties.name = label;

        const auto switch_group = SwitchGroup::create();

        switch_group->addChild(vsg::Mask{MASK_SCENE | MASK_CLICKABLE},
            paged_lod_it->second);

        vsg::vec3 rotation_rad = rotation_deg;

        rotation_rad.x = -vsg::radians(rotation_deg.x);
        rotation_rad.y = -vsg::radians(rotation_deg.y);
        rotation_rad.z = -vsg::radians(rotation_deg.z);

        const vsg::mat4 rotate_x = vsg::rotate(
            rotation_rad.x, vsg::vec3(1.0f, 0.0f, 0.0f));

        const vsg::mat4 rotate_y = vsg::rotate(
            rotation_rad.y, vsg::vec3(0.0f, 1.0f, 0.0f));

        const vsg::mat4 rotate_z = vsg::rotate(
            rotation_rad.z, vsg::vec3(0.0f, 0.0f, 1.0f));

        const vsg::mat4 translate = vsg::translate(translation);

        const auto matrix_transform = vsg::MatrixTransform::create();
        matrix_transform->addChild(switch_group);
        matrix_transform->matrix = translate * rotate_z * rotate_y * rotate_x;
        matrix_transform->setValue("properties", properties);

        this->addChild(vsg::MASK_ALL, matrix_transform);
    }
}

bool Route::load_topology()
{
    const FileSystem& fs = FileSystem::getInstance();

    const auto cfg_path = fs.combinePath(directory,
        "topology", "models-config.xml");

    QString tmp_qstr = cfg_path.c_str();

    CfgReader cfg;
    if (!cfg.load(tmp_qstr))
    {
        // TODO: Replace on Journal
        std::fprintf(stderr, "Failed to load %s\n", cfg_path.c_str());

        return false;
    }

    const QString section_name = "Models";

    tmp_qstr = "";
    if (!cfg.getString(section_name, "SignalModelsDir", tmp_qstr))
    {
        // TODO: Replace on Journal
        std::fprintf(stderr, "Failed to find field \"SignalModelsDir\" "
            "in section %s in %s\n", section_name.toStdString().c_str(),
            cfg_path.c_str());

        return false;
    }

    const std::string models_dir_name = tmp_qstr.toStdString();

    const std::string models_dir = fs.combinePath(fs.getDataDir(),
        "models", models_dir_name);

    // TODO: Replace on Journal
    std::printf("Signals directory: %s\n", models_dir.c_str());

    topology = std::make_unique<Topology>();

    const auto directory_stem = std::filesystem::path(directory).stem();

    if (!topology->load(directory_stem.string().c_str()))
    {
        // TODO: Replace on Journal
        std::fputs("Failed to load topology\n", stderr);

        return false;
    }

    PagedLodMap paged_lods;

    const signals_data_t* const signals_data = topology->getSignalsData();
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

            const auto paged_lod_it = paged_lods.find(signal_model_path);
            if (paged_lod_it == paged_lods.cend())
            {
                const auto new_paged_lod = vsg::PagedLOD::create();
                new_paged_lod->filename = signal_model_path;

                new_paged_lod->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0),
                    settings.view_distance);

                new_paged_lod->children.front() = {0.1, nullptr};
                new_paged_lod->options = options;

                paged_lods[signal_model_path] = new_paged_lod;

                paged_lod = new_paged_lod;
            }
            else
            {
                paged_lod = paged_lod_it->second;
            }

            const auto switch_group = SwitchGroup::create();

            switch_group->addChild(vsg::Mask{MASK_SCENE | MASK_CLICKABLE},
                paged_lod);

            signal->calcPosition();

            const auto pos = to_vsg_vec3(signal->getPos());
            const auto right = to_vsg_vec3(signal->getRight());
            const auto orth = to_vsg_vec3(signal->getOrth());
            const auto up = to_vsg_vec3(signal->getUp());

            const vsg::mat4 translate = vsg::translate(pos);

            const vsg::mat4 rotate = {
                 right.x, -orth.x,  up.x,  0.0,
                -right.y,  orth.y,  up.y,  0.0,
                 right.z,  orth.z,  up.z,  0.0,
                    0.0,      0.0,   0.0,  1.0
            };

            ObjectProperties properties;
            properties.name = signal_model_name;

            const auto matrix_transform = vsg::MatrixTransform::create();
            matrix_transform->addChild(switch_group);
            matrix_transform->matrix = translate * rotate;
            matrix_transform->setValue("properties", properties);

            this->addChild(vsg::MASK_ALL, matrix_transform);
        }
    };

    load_signals(signals_data->line_signals);
    load_signals(signals_data->enter_signals);
    load_signals(signals_data->exit_signals);

    return true;
}
