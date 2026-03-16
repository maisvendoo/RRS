#include "SceneGraph.h"

#include "EditorContext.h"
#include "Mask.h"
#include "Route.h"

#include <vsg/app/Viewer.h>
#include <vsg/core/Mask.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/lighting/AmbientLight.h>

#include <cassert>

SceneGraph::SceneGraph(EditorContext& context)
    : context(context)
{
    ambient_light = vsg::AmbientLight::create();

    this->addChild(vsg::Mask{MASK_SCENE}, ambient_light);
}

void SceneGraph::load_route()
{
    context.route = Route::create(context);

    const auto compile_result = context.viewer->compileManager->compile(context.route);

    this->addChild(vsg::MASK_ALL, context.route);

    vsg::updateViewer(*context.viewer, compile_result);
}
