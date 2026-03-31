#ifndef ROUTE_H
#define ROUTE_H

#include "PagedLodMap.h"
#include "SwitchGroup.h"

#include <vsg/core/Inherit.h>

struct EditorContext;

class Route : public vsg::Inherit<SwitchGroup, Route>
{
public:
    Route(EditorContext& context);

private:
    bool load_objects_ref();
    bool load_route_map();
    void load_static_objects(const PagedLodMap& paged_lods);

    bool load_topology();

private:
    EditorContext& context;
};

#endif // ROUTE_H
