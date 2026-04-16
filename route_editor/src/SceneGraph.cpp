#include "SceneGraph.h"

#include "EditorContext.h"
#include "Mask.h"
#include "Route.h"

#include <vsg/core/Mask.h>
#include <vsg/lighting/AmbientLight.h>

SceneGraph::SceneGraph(EditorContext& context)
    : context_(context)
{
    ambient_light_ = vsg::AmbientLight::create();
    this->addChild(vsg::Mask{MASK_SCENE}, ambient_light_);
}

void SceneGraph::load_route()
{
    context_.route = Route::create(context_);

    std::lock_guard<std::mutex> lock_guard(context_.compile_infos_mutex);
    context_.compile_infos.emplace_back(CompileInfo{
        context_.scene_graph, context_.route, vsg::MASK_ALL});
}
