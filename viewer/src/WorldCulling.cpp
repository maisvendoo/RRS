#include "WorldCulling.h"

#include <vsg/nodes/Node.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/CullGroup.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
WorldCulling::WorldCulling(double in_tile_size_0, double in_tile_size_1)
{
    world_root = vsg::Group::create();

    if (in_tile_size_0 > 0.0)
    {
        tile_size_0 = in_tile_size_0;
    }

    if (in_tile_size_1 > tile_size_0)
    {
        tile_size_1 = in_tile_size_1;
    }
    else
    {
        tile_size_1 = 8.0 * tile_size_0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WorldCulling::add(const vsg::dvec3 point, vsg::ref_ptr<vsg::Node> node)
{
    tile_index_t index_1 = get_index(tile_size_1, point);
    auto itr_1 = tiles.find(index_1);
    if (itr_1 == tiles.end())
    {
        const vsg::dsphere bound = get_bound(tile_size_1, index_1);
        const vsg::ref_ptr<vsg::CullGroup> tile = vsg::CullGroup::create(bound);
        world_root->addChild(tile);

        itr_1 = tiles.emplace(index_1, std::make_pair(tile, tile_map_0_t{})).first;
    }
    tile_map_0_t& tiles_0 = itr_1->second.second;

    tile_index_t index_0 = get_index(tile_size_0, point);
    auto itr_0 = tiles_0.find(index_0);
    if (itr_0 == tiles_0.end())
    {
        const vsg::dsphere bound = get_bound(tile_size_0, index_0);
        const vsg::ref_ptr<vsg::CullGroup> tile = vsg::CullGroup::create(bound);
        itr_1->second.first->addChild(tile);

        itr_0 = tiles_0.emplace(index_0, tile).first;
    }

    itr_0->second->addChild(node);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
WorldCulling::tile_index_t WorldCulling::get_index(const double tile_size, const vsg::dvec3 point) const
{
    return {
        static_cast<int>(std::round(point.x / tile_size)),
        static_cast<int>(std::round(point.y / tile_size)),
        static_cast<int>(std::round(point.z / tile_size))
    };
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::dsphere WorldCulling::get_bound(const double tile_size, const tile_index_t index) const
{
    int x, y, z;
    std::tie(x, y, z) = index;
    return {
        x * tile_size,
        y * tile_size,
        z * tile_size,
        0.875 * tile_size // 0,875 - чуть больше половины диагонали куба
    };
}
