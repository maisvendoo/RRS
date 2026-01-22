#include "SceneGraph.h"

#include "Mask.h"
#include "Route.h"

#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/lighting/AmbientLight.h>

SceneGraph::SceneGraph()
{
    route = Route::create();
    ambient_light = vsg::AmbientLight::create();

    this->addChild(vsg::MASK_ALL, route);
    this->addChild(vsg::Mask{MASK_SCENE}, ambient_light);
}

vsg::ref_ptr<Route> SceneGraph::get_route() const
{
    return route;
}
