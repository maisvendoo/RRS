//------------------------------------------------------------------------------
//
//      Track irregularities profile for RRS
//      Track condition levels
//
//------------------------------------------------------------------------------

#include    "track-condition.h"

#include    <cstring>

namespace track
{

double conditionScale(Condition condition)
{
    switch (condition)
    {
    case Condition::Excellent:
        return 0.15;
    case Condition::Good:
        return 0.35;
    case Condition::Normal:
        return 1.0;
    case Condition::Worn:
        return 1.8;
    case Condition::Poor:
        return 3.0;
    case Condition::Critical:
        return 5.0;
    }

    return 1.0;
}

Condition conditionFromString(const char* name, bool* ok)
{
    if (ok != nullptr)
        *ok = true;

    if (std::strcmp(name, "excellent") == 0)
        return Condition::Excellent;
    if (std::strcmp(name, "good") == 0)
        return Condition::Good;
    if (std::strcmp(name, "normal") == 0)
        return Condition::Normal;
    if (std::strcmp(name, "worn") == 0)
        return Condition::Worn;
    if (std::strcmp(name, "poor") == 0)
        return Condition::Poor;
    if (std::strcmp(name, "critical") == 0)
        return Condition::Critical;

    if (ok != nullptr)
        *ok = false;
    return Condition::Normal;
}

const char* conditionToString(Condition condition)
{
    switch (condition)
    {
    case Condition::Excellent:  return "excellent";
    case Condition::Good:       return "good";
    case Condition::Normal:     return "normal";
    case Condition::Worn:       return "worn";
    case Condition::Poor:       return "poor";
    case Condition::Critical:   return "critical";
    }

    return "normal";
}

} // namespace track
