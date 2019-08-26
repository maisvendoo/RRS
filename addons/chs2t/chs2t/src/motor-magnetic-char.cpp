//------------------------------------------------------------------------------
//
//      Магистральный пассажирский электровоз постоянного тока ЧС2т.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Николай Авилкин (avilkin.nick)
//
//      Дата: 21/08/2019
//
//------------------------------------------------------------------------------

#include    "motor-magnetic-char.h"

#include    <fstream>
#include    <sstream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MotorMagneticChar::MotorMagneticChar()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MotorMagneticChar::~MotorMagneticChar()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MotorMagneticChar::load(const std::string &path)
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

            ss >> p.current >> p.value;

            points.push_back(p);
        }

        double cphi = getValue(500.0);
        double zu = 0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double MotorMagneticChar::getValue(double I)
{
    if (I >= 0)
        return interpolate(I);
    else
        return -interpolate(-I);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MotorMagneticChar::point_t MotorMagneticChar::findPoint(double I, point_t &next_point)
{
    point_t point;

    size_t left_idx = 0;
    size_t right_idx = points.size() - 2;
    size_t idx = (left_idx + right_idx) / 2;

    while (idx != left_idx)
    {
        point_t p = points[idx];

        if (I <= p.current)
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
double MotorMagneticChar::interpolate(double I)
{
    point_t p1;
    point_t p0 = findPoint(I, p1);

    return p0.value + (p1.value - p0.value) * (I - p0.current) / (p1.current - p0.current);
}
