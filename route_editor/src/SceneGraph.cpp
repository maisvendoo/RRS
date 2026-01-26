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

SceneGraph::SceneGraph(
    const settings_t& settings,
    vsg::ref_ptr<vsg::Options> options
)
    : settings(settings)
    , options(options)
{
    assert(options);

    ambient_light = vsg::AmbientLight::create();

    this->addChild(vsg::Mask{MASK_SCENE}, ambient_light);
}

void SceneGraph::load_route(
    vsg::observer_ptr<vsg::Viewer> observer_viewer,
    const std::string& directory
)
{
    assert(observer_viewer);

    route = Route::create(settings, options, directory);

    const auto viewer = observer_viewer.ref_ptr();
    const auto compile_manager = viewer->compileManager;
    const auto compile_result = compile_manager->compile(route);

    this->addChild(vsg::MASK_ALL, route);

    vsg::updateViewer(*viewer, compile_result);
}

vsg::ref_ptr<Route> SceneGraph::get_route() const
{
    return route;
}
