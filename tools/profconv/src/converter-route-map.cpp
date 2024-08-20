#include    "converter.h"

#include    <QFile>
#include    <iostream>
#include    <sstream>

#include    "path-utils.h"


void add_element(float x, std::vector<float> &array)
{
    float eps = 1.0f;

    if (array.size() != 0)
    {
        float last = *(array.end() - 1);

        if ( abs(last - x) >= eps )
            array.push_back(x);
    }
    else
    {
        array.push_back(x);
    }
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readRouteMAP(const std::string &path, zds_route_map_data_t &map_data)
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
    return readRouteMAP(stream, map_data);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readRouteMAP(QTextStream &stream, zds_route_map_data_t &map_data)
{
    bool may_add_info_to_last = false;
    while (!stream.atEnd())
    {
        QString line = stream.readLine();

        if (line.isEmpty())
            continue;

        // Пустое название объекта
        if (*(line.begin()) == ',')
            continue;

        // Строка с объектом заканчивается точкой с запятой, а если нет,
        // это может быть строка с информацией об объекте из предыдущей строки
        if (*(line.end() - 1) != ';')
        {
            if (may_add_info_to_last)
            {
                map_data.back()->obj_info = line.toStdString();
                may_add_info_to_last = false;
            }
            continue;
        }

        // Удаляем завершающую точку с запятой и разделяем строку на данные
        QStringList tokens = line.removeLast().split(',');
        if (tokens.size() < 7)
            continue;

        // Читаем информацию об объекте
        zds_object_position_t zds_object = zds_object_position_t();
        zds_object.obj_name = tokens[0].toStdString();
        zds_object.position.x = tokens[1].toDouble();
        zds_object.position.y = tokens[2].toDouble();
        zds_object.position.z = tokens[3].toDouble();
        zds_object.attitude.x = tokens[4].toDouble();
        zds_object.attitude.y = tokens[5].toDouble();
        zds_object.attitude.z = tokens[6].toDouble();

        map_data.push_back(new zds_object_position_t(zds_object));
        may_add_info_to_last = true;
    }

    return (!map_data.empty());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::findNeutralInsertions(std::vector<neutral_insertion_t> ni)
{
    std::vector<float> begins;
    std::vector<float> ends;

    for (auto zds_object : route_map_data)
    {
        if (zds_object->obj_name == "nvne")
        {
            neutral_insertion_t n_insertion;

            float begin_coord = 0;
            zds_track_t track = getNearestTrack(zds_object->position, tracks_data1, begin_coord);

            add_element(begin_coord, begins);
        }

        if (zds_object->obj_name == "nvke")
        {
            neutral_insertion_t n_insertion;

            float end_coord = 0;
            zds_track_t track = getNearestTrack(zds_object->position, tracks_data1, end_coord);

            add_element(end_coord, ends);
        }
    }

    std::sort(begins.begin(), begins.end(), std::less<float>());
    std::sort(ends.begin(), ends.end(), std::less<float>());

    if (begins.size() == ends.size())
    {
        float max_len = 500.0f;

        for (size_t i = 0; i < begins.size(); ++i)
        {
            neutral_insertion_t n_ins;

            n_ins.begin_coord = begins[i];
            n_ins.end_coord = ends[i];
            n_ins.length = abs(n_ins.end_coord - n_ins.begin_coord);

            if (n_ins.length <= max_len)
                ni.push_back(n_ins);
        }
    }

    return ni.size() != 0;
}
