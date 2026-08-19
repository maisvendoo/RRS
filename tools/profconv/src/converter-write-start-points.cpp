#include    "converter.h"

#include    <QFile>
#include    "path-utils.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeStartPoints(const start_point_data_t &start_points)
{
    std::string path = compinePath(topologyDir, FILE_START_POINT);

    QFile file_old(QString(path.c_str()));
    if (file_old.exists())
    {
        std::string backup = FILE_BACKUP_PREFIX + FILE_START_POINT + FILE_BACKUP_EXTENTION;
        file_old.rename( QString(compinePath(topologyDir, backup).c_str()) );
    }

    if (start_points.empty())
        return;

    QFile file(QString(path.c_str()));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
/*
    stream << "#Станция"
           << DELIMITER_SYMBOL << "#Траектория"
           << DELIMITER_SYMBOL << "#Напр."
           << DELIMITER_SYMBOL << "#Коорд."
           << DELIMITER_SYMBOL << "#Жд-пикетаж"
           << "\n";
*/
    for (auto start_point : start_points)
    {
        stream << start_point->name.c_str()
               << DELIMITER_SYMBOL << start_point->trajectory_name.c_str()
               << DELIMITER_SYMBOL << start_point->direction
               << DELIMITER_SYMBOL << start_point->trajectory_coord
               << DELIMITER_SYMBOL << static_cast<int>(round(start_point->railway_coord))
               << "\n";
    }

    file.close();
}
