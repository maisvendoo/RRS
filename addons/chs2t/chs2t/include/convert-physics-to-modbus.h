#ifndef CONVERTPHYSICSTOMODBUS_H
#define CONVERTPHYSICSTOMODBUS_H

#include    <QString>
#include    <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class PhysToModbus
{
public:
    PhysToModbus();

    ~PhysToModbus();

    double getModbus(double physValue) { return interpolate(physValue); }

    void load(const std::string &path);

private:

    struct point_t
    {
        double  current;
        double  value;

        point_t()
            : current(0.0)
            , value(0.0)
        {

        }
    };

    double physValue;

    std::vector<point_t> points;

    point_t findPoint(double physValue, point_t &next_point);

    double  interpolate(double physValue);

    double calculateModbus(double physValue);
};

#endif // CONVERTPHYSICSTOMODBUS_H
