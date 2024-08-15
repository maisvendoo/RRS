#include    "converter.h"

#include    <QFile>

#include    "path-utils.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeTopologyTrajectory(const trajectory_t* trajectory)
{
    std::string path = compinePath(toNativeSeparators(trajectoriesDir), trajectory->name + FILE_EXTENTION);

    QFile file_old(QString(path.c_str()));
    if (file_old.exists())
        file_old.rename( QString((path + FILE_BACKUP_EXTENTION).c_str()) );

    QFile file(QString(path.c_str()));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream.setRealNumberNotation(QTextStream::FixedNotation);

    for (auto point = trajectory->points.begin(); point != trajectory->points.end(); ++point)
    {
        stream                  << point->point.x
            << DELIMITER_SYMBOL << point->point.y
            << DELIMITER_SYMBOL << point->point.z
            << DELIMITER_SYMBOL << static_cast<int>(round(point->railway_coord))
            << DELIMITER_SYMBOL << point->trajectory_coord
            << "\n";
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeTopologyConnectors(const std::string &filename, const route_connectors_t &connectors)
{

}
