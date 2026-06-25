#include "editor/Route.h"

#include "editor/RouteMapTransform.h"

#include <Journal.h>
#include <core/string_funcs.h>
#include <filesystem.h>

#include <vsg/core/ref_ptr.h>
#include <vsg/io/stream.h>
#include <vsg/maths/common.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>

#include <QString>

#include <algorithm>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

Route::Route(
    const std::unique_ptr<ObjectManager>& object_manager,
    const double& view_distance,
    const vsg::ref_ptr<vsg::Options>& vsg_options
)
    : object_manager(object_manager)
    , view_distance(view_distance)
    , vsg_options(vsg_options)
{
}

void Route::start_load(const std::string& route_dir)
{
    load_static_objects(route_dir);
    // load_static_objects_thread = std::thread([this, &route_dir]() -> void {
    //     load_static_objects(route_dir);
    // });

    load_topology_thread = std::thread([this, &route_dir]() -> void {
        load_topology(route_dir);
    });
}

void Route::join_threads()
{
    if (load_static_objects_thread.joinable())
    {
        load_static_objects_thread.join();
    }

    if (load_topology_thread.joinable())
    {
        load_topology_thread.join();
    }
}

void Route::load_static_objects(const std::string& route_dir)
{
    load_objects_ref(route_dir);
    load_route_map(route_dir);
}

bool Route::load_objects_ref(const std::string& route_dir)
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string objects_ref_path = fs.combinePath(route_dir, "objects.ref");

    std::ifstream objects_ref_file(objects_ref_path);
    if (!objects_ref_file)
    {
        Journal::instance()->error(QString("Failed to open objects.ref file %1")
            .arg(to_qstring(objects_ref_path)));

        return false;
    }

    Journal::instance()->info(QString("objects.ref file %1 is successfully opened")
        .arg(to_qstring(objects_ref_path)));

    std::string line;
    while (std::getline(objects_ref_file, line))
    {
        std::istringstream iss(std::move(line));
        Label label;
        RelativePath relative_path;
        if (iss >> label >> relative_path)
        {
            objects_ref[label] = relative_path;
        }
    }

    Journal::instance()->info(QString("objects.ref file %1 is successfully loaded")
        .arg(to_qstring(objects_ref_path)));

    print_objects_ref_in_journal();

    return true;
}

void Route::print_objects_ref_in_journal() const
{
    Journal::instance()->info(QString("%1Relative path").arg("Label", -32));

    for (const auto& [label, relative_path] : objects_ref)
    {
        Journal::instance()->info(QString("%1%2")
            .arg(to_qstring(label), -32)
            .arg(to_qstring(relative_path)));
    }
}

bool Route::load_route_map(const std::string& route_dir)
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string route_map_path = fs.combinePath(route_dir,
        "topology", "map", "route1.map");

    std::ifstream route_map_file(route_map_path);
    if (!route_map_file)
    {
        Journal::instance()->error(QString("Failed to open route1.map file %1")
            .arg(to_qstring(route_map_path)));

        return false;
    }

    Journal::instance()->info(QString("route1.map file %1 is successfully opened")
        .arg(to_qstring(route_map_path)));

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
        Label label;
        RouteMapTransform transform;
        if (iss >> label >> transform.translation >> transform.rotation)
        {
            if (objects_ref.find(label) == objects_ref.end())
            {
                Journal::instance()->warning(QString("Failed to find label %1 in objects.ref")
                    .arg(to_qstring(label)));
            }
            else
            {
                route_map[label].push_back(transform);
            }
        }
    }

    Journal::instance()->info(QString("route1.map file %1 is successfully loaded")
        .arg(to_qstring(route_map_path)));

    print_route_map_in_journal();

    for (const auto& [label, transforms] : route_map)
    {
        const auto paged_lod = vsg::PagedLOD::create();
        paged_lod->filename = fs.combinePath(route_dir, objects_ref[label]);
        paged_lod->bound = {vsg::dvec3(0.0, 0.0, 0.0), view_distance};
        paged_lod->children.front() = {0.1, nullptr};
        paged_lod->options = vsg_options;

        for (const auto& transform : transforms)
        {
            const auto rotation = -transform.rotation;

            const auto matrix_transform = vsg::MatrixTransform::create();
            matrix_transform->matrix = vsg::translate(transform.translation) *
                vsg::rotate(vsg::radians(rotation.z), vsg::dvec3(0.0, 0.0, 1.0)) *
                vsg::rotate(vsg::radians(rotation.y), vsg::dvec3(0.0, 1.0, 0.0)) *
                vsg::rotate(vsg::radians(rotation.x), vsg::dvec3(1.0, 0.0, 0.0));
            matrix_transform->addChild(paged_lod);
            temp_transforms.emplace_back(matrix_transform);
        }
    }

    return true;
}

void Route::print_route_map_in_journal() const
{
    Journal::instance()->info(QString("%1%2Rotation")
        .arg("Label", -32)
        .arg("Translation", -40));

    for (const auto& [label, transforms] : route_map)
    {
        for (const RouteMapTransform& transform : transforms)
        {
            Journal::instance()->info(QString("%1{%2, %3, %4}    {%5, %6, %7}")
                .arg(label, -32)
                .arg(transform.translation.x, 10, 'f', 3)
                .arg(transform.translation.y, 10, 'f', 3)
                .arg(transform.translation.z, 10, 'f', 3)
                .arg(transform.rotation.x, 10, 'f', 3)
                .arg(transform.rotation.y, 10, 'f', 3)
                .arg(transform.rotation.z, 10, 'f', 3));
        }
    }
}

void Route::load_topology(const std::string& route_dir)
{
}
