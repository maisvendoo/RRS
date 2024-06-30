#include    "linear-interpolation.h"

#include    <fstream>
#include    <sstream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LinearInterpolation::LinearInterpolation()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LinearInterpolation::~LinearInterpolation()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LinearInterpolation::load(const std::string &path)
{
    std::ifstream stream(path.c_str(), std::ios::in);

    if (stream.is_open())
    {
        while (!stream.eof())
        {
            std::string line = "";
            std::getline(stream, line);

            std::istringstream ss(line);

            point_t p;

            ss >> p.parameter >> p.value;

            points.push_back(p);
        }        
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double LinearInterpolation::getValue(double parameter)
{
    if (parameter >= 0)
        return interpolate(parameter);
    else
        return -interpolate(-parameter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LinearInterpolation::point_t LinearInterpolation::findPoint(double parameter, point_t &next_point)
{
    point_t point;

    size_t left_idx = 0;
    size_t right_idx = points.size() - 2;
    size_t idx = (left_idx + right_idx) / 2;

    while (idx != left_idx)
    {
        point_t p = points[idx];

        if (parameter <= p.parameter)
            right_idx = idx;
        else
            left_idx = idx;

        idx = (left_idx + right_idx) / 2;
    }

    point = points[idx];
    next_point = points[idx+1];

    return point;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double LinearInterpolation::interpolate(double parameter)
{
    if (points.empty())
        return 0;

    point_t p1;
    point_t p0 = findPoint(parameter, p1);

    return p0.value + (p1.value - p0.value) * (parameter - p0.parameter) / (p1.parameter - p0.parameter);
}
