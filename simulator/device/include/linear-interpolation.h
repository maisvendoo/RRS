#ifndef     LINEAR_INTERPOLATION_H
#define     LINEAR_INTERPOLATION_H

#include    <QObject>
#include    <vector>

#include    "device-export.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT LinearInterpolation
{
public:

    LinearInterpolation();

    ~LinearInterpolation();

    void load(const std::string &path);

    double getValue(double parameter);

private:

    struct point_t
    {
        double  parameter;
        double  value;

        point_t()
            : parameter(0.0)
            , value(0.0)
        {

        }
    };

    std::vector<point_t> points;

    point_t findPoint(double I, point_t &next_point);

    double  interpolate(double I);
};

#endif // LINEAR_INTERPOLATION_H
