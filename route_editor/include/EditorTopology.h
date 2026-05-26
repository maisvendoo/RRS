#ifndef EDITOR_TOPOLOGY_H
#define EDITOR_TOPOLOGY_H

#include <QStringList>

#include <string>

class EditorTopology
{
public:
    bool load(const std::string& route_name);

private:
    QStringList get_trajectory_names(const std::string& route_dir);
};

#endif // EDITOR_TOPOLOGY_H
