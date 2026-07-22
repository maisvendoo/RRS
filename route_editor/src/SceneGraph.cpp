#include "SceneGraph.h"

#include "EditorContext.h"
#include "Mask.h"
#include "Route.h"

#include <vsg/core/Mask.h>
#include <vsg/lighting/AmbientLight.h>

SceneGraph::SceneGraph(EditorContext& context, const camera_settings_t& camera_settings)
    : context_(context)
    , camera_settings(camera_settings)
{
    ambient_light_ = vsg::AmbientLight::create();
    this->addChild(vsg::Mask{MASK_SCENE}, ambient_light_);
}

void SceneGraph::load_route()
{
    context_.route = Route::create(context_, camera_settings);

    context_.compile_infos.emplace_back(CompileInfo{
        context_.scene_graph, context_.route, vsg::MASK_ALL});
}
