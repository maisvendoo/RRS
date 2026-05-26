#include "EditorTopology.h"

#include <Journal.h>
#include <core/string_funcs.h>
#include <filesystem.h>

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QString>
#include <QStringList>

#include <string>

bool EditorTopology::load(const std::string& route_name)
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string route_dir = fs.combinePath(
        fs.getRouteRootDir(),
        route_name
    );

    const QStringList trajectory_names = get_trajectory_names(route_dir);
    if (trajectory_names.isEmpty())
    {
        Journal::instance()->error("Could not find trajectories");
        return false;
    }

    return true;
}

QStringList EditorTopology::get_trajectory_names(const std::string& route_dir)
{
    const FileSystem& fs = FileSystem::getInstance();

    const QString trajectories_dir_path = to_qstring(
        fs.combinePath(route_dir, "topology", "trajectories")
    );

    Journal::instance()->info("Check trajectories in " + trajectories_dir_path);

    QDirIterator trajectory_files(
        trajectories_dir_path,
        QStringList() << "*.traj",
        QDir::Files | QDir::NoDotAndDotDot
    );

    QStringList trajectory_names;

    while (trajectory_files.hasNext())
    {
        const QString trajectory_path = trajectory_files.next();
        Journal::instance()->info("Found trajectory " + trajectory_path);
        const QFileInfo file_info(trajectory_path);
        trajectory_names << file_info.baseName();
    }

    return trajectory_names;
}
