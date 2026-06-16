#include "editor/Route.h"

#include <Journal.h>
#include <core/string_funcs.h>
#include <filesystem.h>

#include <QString>

#include <fstream>
#include <sstream>
#include <string>
#include <thread>

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
    for (const auto& [label, relative_path] : objects_ref)
    {
        Journal::instance()->info(QString("label: \"%1\"; relative_path: \"%2\"")
            .arg(to_qstring(label)).arg(to_qstring(relative_path)));
    }
}

void Route::load_route_map(const std::string& route_dir)
{
}

void Route::load_topology(const std::string& route_dir)
{
}
