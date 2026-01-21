#include "SceneGraph.h"

#include "Route.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/lighting/AmbientLight.h>

SceneGraph::SceneGraph()
{
    route = Route::create();
    ambient_light = vsg::AmbientLight::create();

    this->addChild(route);
    this->addChild(ambient_light);
}

vsg::ref_ptr<Route> SceneGraph::get_route() const
{
    return route;
}
