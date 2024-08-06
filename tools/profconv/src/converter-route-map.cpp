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
bool ZDSimConverter::readRouteMAP(const std::string &path,
                                  std::vector<neutral_insertion_t> ni)
{
    if (path.empty())
        return false;

    std::ifstream stream(path.c_str(), std::ios::in);

    if (!stream.is_open())
    {
        std::cout << "File " << path << " not opened" << std::endl;
        return false;
    }

    return findNeutralInsertions(stream, ni);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::findNeutralInsertions(std::ifstream &stream,
                                           std::vector<neutral_insertion_t> ni)
{
    std::vector<float> begins;
    std::vector<float> ends;

    while (!stream.eof())
    {
        std::string line;
        std::getline(stream, line);

        line = delete_symbol(line, '\r');
        std::replace(line.begin(), line.end(), ',', ' ');

        if (line.empty())
            continue;

        if (*(line.end() - 1) != ';')
            continue;

        if (line.at(0) == ',')
            continue;

        std::string obj_name = "";

        std::istringstream ss(line);

        dvec3 pos;

        ss >> obj_name >> pos.x >> pos.y >> pos.z;

        if (obj_name == "nvne")
        {
            neutral_insertion_t n_insertion;

            float begin_coord = 0;
            zds_track_t track = getNearestTrack(pos, tracks_data1, begin_coord);

            add_element(begin_coord, begins);
        }

        if (obj_name == "nvke")
        {
            neutral_insertion_t n_insertion;

            float end_coord = 0;
            zds_track_t track = getNearestTrack(pos, tracks_data1, end_coord);

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
