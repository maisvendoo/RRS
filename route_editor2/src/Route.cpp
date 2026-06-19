#include "editor/Route.h"

#include "editor/RouteMapTransformation.h"

#include <Journal.h>
#include <core/string_funcs.h>
#include <filesystem.h>

#include <vsg/io/stream.h>
#include <vsg/maths/vec3.h>

#include <QString>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

Route::Route(const std::unique_ptr<ObjectManager>& object_manager)
    : object_manager(object_manager)
{
}

void Route::start_load(const std::string& route_dir)
{
    load_static_objects_thread = std::thread([this, &route_dir]() -> void {
        load_static_objects(route_dir);
    });

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
        RouteMapTransformation transformation;
        if (iss >> label >> transformation.translation >> transformation.rotation)
        {
            route_map[label].push_back(transformation);
        }
    }

    Journal::instance()->info(QString("route1.map file %1 is successfully loaded")
        .arg(to_qstring(route_map_path)));

    print_route_map_in_journal();

    return true;
}

void Route::print_route_map_in_journal() const
{
    Journal::instance()->info(QString("%1%2Rotation")
        .arg("Label", -32)
        .arg("Translation", -40));

    for (const auto& [label, transformations] : route_map)
    {
        for (const RouteMapTransformation& transformation : transformations)
        {
            Journal::instance()->info(QString("%1{%2, %3, %4}    {%5, %6, %7}")
                .arg(label, -32)
                .arg(transformation.translation.x, 10, 'f', 3)
                .arg(transformation.translation.y, 10, 'f', 3)
                .arg(transformation.translation.z, 10, 'f', 3)
                .arg(transformation.rotation.x, 10, 'f', 3)
                .arg(transformation.rotation.y, 10, 'f', 3)
                .arg(transformation.rotation.z, 10, 'f', 3));
        }
    }
}

void Route::load_topology(const std::string& route_dir)
{
}
