//------------------------------------------------------------------------------
//
//      osm-import: геометрия - проекция и упрощение
//
//------------------------------------------------------------------------------

#include "osm-geometry.h"

#include <cmath>

namespace osm
{

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static double point_segment_distance(const LocalPoint& p,
                                     const LocalPoint& a,
                                     const LocalPoint& b)
{
    const double dx = b.x - a.x;
    const double dy = b.y - a.y;
    const double length_sq = dx * dx + dy * dy;

    if (length_sq < 1.0e-12)
    {
        const double ex = p.x - a.x;
        const double ey = p.y - a.y;
        return std::sqrt(ex * ex + ey * ey);
    }

    double t = ((p.x - a.x) * dx + (p.y - a.y) * dy) / length_sq;
    t = std::max(0.0, std::min(1.0, t));

    const double cx = a.x + t * dx;
    const double cy = a.y + t * dy;

    const double ex = p.x - cx;
    const double ey = p.y - cy;

    return std::sqrt(ex * ex + ey * ey);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static void rdp(const std::vector<LocalPoint>& points,
                std::size_t first, std::size_t last,
                double epsilon, std::vector<bool>& keep)
{
    if (last <= first + 1)
    {
        return;
    }

    double max_dist = 0.0;
    std::size_t index = first;

    for (std::size_t i = first + 1; i < last; ++i)
    {
        const double dist = point_segment_distance(points[i],
                                                   points[first],
                                                   points[last]);
        if (dist > max_dist)
        {
            max_dist = dist;
            index = i;
        }
    }

    if (max_dist > epsilon)
    {
        keep[index] = true;
        rdp(points, first, index, epsilon, keep);
        rdp(points, index, last, epsilon, keep);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<LocalPoint> simplify_polyline(const std::vector<LocalPoint>& points,
                                          double epsilon_m)
{
    if (points.size() <= 2)
    {
        return points;
    }

    std::vector<bool> keep(points.size(), false);
    keep.front() = true;
    keep.back() = true;

    rdp(points, 0, points.size() - 1, epsilon_m, keep);

    std::vector<LocalPoint> result;
    result.reserve(points.size());

    for (std::size_t i = 0; i < points.size(); ++i)
    {
        if (keep[i])
        {
            result.push_back(points[i]);
        }
    }

    return result;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double polyline_length(const std::vector<LocalPoint>& points)
{
    double length = 0.0;

    for (std::size_t i = 1; i < points.size(); ++i)
    {
        const double dx = points[i].x - points[i - 1].x;
        const double dy = points[i].y - points[i - 1].y;
        length += std::sqrt(dx * dx + dy * dy);
    }

    return length;
}

} // namespace osm
