#include    "converter.h"

#include    <QFile>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readObjectsRef(const std::string &path, zds_objects_ref_data_t &objects_data)
{
    if (path.empty())
        return false;

    QString data = fileToQString(path);
    if (data.isEmpty())
    {
        std::cout << "File " << path << " not opened" << std::endl;
        return false;
    }

    QTextStream stream(&data);
    return readObjectsRef(stream, objects_data);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readObjectsRef(QTextStream &stream, zds_objects_ref_data_t &objects_data)
{
    while (!stream.atEnd())
    {
        QString line = stream.readLine();

        // Пустая строка
        if (line.isEmpty())
            continue;

        // Строка-комментарий начинается с точки с запятой
        if (*(line.begin()) == ';')
            continue;

        // Разделяем строку по табуляции: имя, путь к модели, путь к текстуре
        QStringList tokens = line.split('\t');
        if (tokens.size() < 3)
            continue;

        // Проверяем наличие модели
        QString model_path = QString(ZDSrouteDir.c_str()) + tokens[1];
        if (!QFile(model_path).exists())
            continue;

        // Проверяем наличие текстуры
        QString texture_path = QString(ZDSrouteDir.c_str()) + tokens[2];
        if (!QFile(texture_path).exists())
            continue;

        zds_object_ref_t* zds_object = new zds_object_ref_t;
        zds_object->object_name = tokens[0].toStdString();
        zds_object->model_path = tokens[1].toStdString();
        zds_object->texture_path = tokens[2].toStdString();

        // Сохраняем с именем, обрезанным до 20 символов (привет Славе Усову!)
        QString object_20symb = tokens[0];
        object_20symb.truncate(20);
        if (objects_data.count(object_20symb.toStdString()) == 0)
            objects_data.insert({object_20symb.toStdString(), zds_object});
        else
            std::cout << "Warning: multiple objects with name=" << object_20symb.toStdString() << std::endl;
    }

    return true;
}
