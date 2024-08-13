#include    "converter.h"

#include    <iostream>
#include    <QFile>

#include    "path-utils.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeTopologyTrajectories(const route_trajectories_t &trajectories)
{
    for (auto it = trajectories.begin(); it != trajectories.end(); ++it)
    {
        std::string path = compinePath(toNativeSeparators(topologyDir), (*it)->name + ".trj");

        QFile file_old(QString(path.c_str()));
        if (file_old.exists())
            file_old.rename( QString((path + ".bak").c_str()) );

        QFile file(QString(path.c_str()));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        QTextStream stream(&file);
        stream.setEncoding(QStringConverter::Utf8);
        stream.setRealNumberNotation(QTextStream::FixedNotation);

        for (auto point = (*it)->points.begin(); point != (*it)->points.end(); ++point)
        {
            stream << point->point.x << ";"
                   << point->point.y << ";"
                   << point->point.z << ";"
                   << static_cast<int>(round(point->railway_coord)) << ";"
                   << point->trajectory_coord << "\n";
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeTopologyConnectors(const std::string &filename, const route_connectors_t &connectors)
{

}
