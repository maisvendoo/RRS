#include    "converter.h"

#include    <QDir>
#include    <QVariant>

#include    "path-utils.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeMap()
{
    if (map_data_objects_no_info.empty() && map_data_objects_with_info.empty())
        return;

    std::string path = compinePath(route1mapDir, FILE_ROUTE1MAP);

    QFile file(QString(path.c_str()));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream.setRealNumberNotation(QTextStream::FixedNotation);

    QChar delimiter_symbol = QChar(',');
    QChar end_symbol = QChar(';');

    for (auto map : {map_data_objects_no_info,
                     map_data_objects_with_info})
    {
        for (auto obj : map)
        {
            stream << obj->obj_name.c_str()
                   << delimiter_symbol << obj->position.x
                   << delimiter_symbol << obj->position.y
                   << delimiter_symbol << obj->position.z
                   << delimiter_symbol << obj->attitude.x
                   << delimiter_symbol << obj->attitude.y
                   << delimiter_symbol << obj->attitude.z
                   << end_symbol << "\n";
            if (obj->obj_info.empty())
            {
                stream << "\n";
            }
            else
            {
                stream << obj->obj_info.c_str() << "\n";
            }
        }
    }

    file.close();
}
