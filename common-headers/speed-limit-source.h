#ifndef SPEED_LIMIT_SOURCE_H
#define SPEED_LIMIT_SOURCE_H

#include <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct speed_limit_interval_t
{
    double begin = 0.0;
    double end = 0.0;
    double speed_kmh = 0.0;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SpeedLimitSource
{
public:
    virtual std::vector<speed_limit_interval_t> getSpeedLimits() const = 0;
    virtual ~SpeedLimitSource() = default;
};

#endif // SPEED_LIMIT_SOURCE_H