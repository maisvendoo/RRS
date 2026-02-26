#include "SceneGraph.h"

#include "Mask.h"
#include "Route.h"

#include <vsg/app/Viewer.h>
#include <vsg/core/Mask.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/lighting/AmbientLight.h>

#include <cassert>
#include <string>

SceneGraph::SceneGraph(const EditorContext& context)
    : context(context)
{
    ambient_light = vsg::AmbientLight::create();

    this->addChild(vsg::Mask{MASK_SCENE}, ambient_light);
}

void SceneGraph::load_route()
{
    route = Route::create(context.settings, context.options, context.route_dir);

    const auto compile_result = context.viewer->compileManager->compile(route);

    this->addChild(vsg::MASK_ALL, route);

    vsg::updateViewer(*context.viewer, compile_result);
}

vsg::ref_ptr<Route> SceneGraph::get_route() const
{
    return route;
}
