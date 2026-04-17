#ifndef WORLD_CULLING_H
#define WORLD_CULLING_H

#include <map>
#include <vsg/core/Inherit.h>
#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/sphere.h>

namespace vsg
{
    class Node;
    class Group;
    class CullGroup;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class WorldCulling final : public vsg::Inherit<vsg::Object, WorldCulling>
{
public:
    WorldCulling(double in_tile_size_0 = 4000.0, double in_tile_size_1 = 32000.0);

    vsg::ref_ptr<vsg::Group> world_root;

    void add(const vsg::dvec3 point, vsg::ref_ptr<vsg::Node> node);

private:

    using tile_index_t = std::tuple<int, int, int>;
    using tile_map_0_t = std::map<tile_index_t, vsg::ref_ptr<vsg::CullGroup>>;
    using tile_map_1_t = std::map<tile_index_t, std::pair<vsg::ref_ptr<vsg::CullGroup>, tile_map_0_t>>;
    tile_map_1_t tiles;

    double tile_size_0 = 4000.0;
    double tile_size_1 = 32000.0;

    tile_index_t get_index(const double tile_size, const vsg::dvec3 point) const;
    vsg::dsphere get_bound(const double tile_size, const tile_index_t index) const;
};

#endif // WORLD_CULLING_H
