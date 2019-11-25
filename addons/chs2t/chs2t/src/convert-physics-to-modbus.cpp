#include "convert-physics-to-modbus.h"

#include    <fstream>
#include    <sstream>

PhysToModbus::PhysToModbus()
{

}

PhysToModbus::~PhysToModbus()
{

}

void PhysToModbus::load(const std::string& path)
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
    }
}

PhysToModbus::point_t PhysToModbus::findPoint(double physValue, PhysToModbus::point_t& next_point)
{
    point_t point;

    size_t left_idx = 0;
    size_t right_idx = points.size() - 2;
    size_t idx = (left_idx + right_idx) / 2;

    while (idx != left_idx)
    {
        point_t p = points[idx];

        if (physValue <= p.current)
            right_idx = idx;
        else
            left_idx = idx;

        idx = (left_idx + right_idx) / 2;
    }

    point = points[idx];
    next_point = points[idx+1];

    return point;
}

double PhysToModbus::interpolate(double physValue)
{
    point_t p1;
    point_t p0 = findPoint(physValue, p1);

    return p0.value + (p1.value - p0.value) * (physValue - p0.current) / (p1.current - p0.current);
}

