#include "SceneGraph.h"

#include "EditorContext.h"
#include "Mask.h"
#include "Route.h"

#include <vsg/core/Mask.h>
#include <vsg/lighting/AmbientLight.h>

SceneGraph::SceneGraph(
    EditorContext& context,
    const camera_settings_t& camera_settings,
    const vsg::ref_ptr<vsg::Options>& vsg_options
)
    : context_(context)
    , camera_settings(camera_settings)
    , vsg_options(vsg_options)
{
    ambient_light_ = vsg::AmbientLight::create();
    this->addChild(vsg::Mask{MASK_SCENE}, ambient_light_);
}

void SceneGraph::load_route()
{
    context_.route = Route::create(context_, camera_settings, vsg_options);

    context_.compile_infos.emplace_back(CompileInfo{
        vsg::ref_ptr(this), context_.route, vsg::MASK_ALL});
}
