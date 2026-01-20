#include "SceneGraph.h"

#include "Mask.h"
#include "Route.h"

#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/lighting/AmbientLight.h>
#include <vsg/nodes/Switch.h>

SceneGraph::SceneGraph()
{
    route = Route::create();

    ambient_light = vsg::AmbientLight::create();

    const auto ambient_switch = vsg::Switch::create();
    ambient_switch->addChild(vsg::Mask{MASK_SCENE}, ambient_light);

    this->addChild(route);
    this->addChild(ambient_switch);
}

vsg::ref_ptr<Route> SceneGraph::get_route() const
{
    return route;
}
