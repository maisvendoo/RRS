#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include "SwitchGroup.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>

#include <string>

class Route;
struct settings_t;

namespace vsg
{

class AmbientLight;
class Options;
class Viewer;

}

class SceneGraph : public vsg::Inherit<SwitchGroup, SceneGraph>
{
public:
    SceneGraph(const settings_t& settings, vsg::ref_ptr<vsg::Options> options);

    void load_route(
        vsg::observer_ptr<vsg::Viewer> observer_viewer,
        const std::string& directory
    );

    vsg::ref_ptr<Route> get_route() const;

private:
    const settings_t& settings;
    vsg::ref_ptr<vsg::Options> options;

    vsg::ref_ptr<Route> route;
    vsg::ref_ptr<vsg::AmbientLight> ambient_light;
};

#endif // SCENE_GRAPH_H
