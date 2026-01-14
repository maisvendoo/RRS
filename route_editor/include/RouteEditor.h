#ifndef ROUTE_EDITOR_H
#define ROUTE_EDITOR_H

#include "CameraHandler.h"
#include "EditorState.h"
#include "KeyboardHandler.h"
#include "MouseHandler.h"
#include "ObjectSelector.h"
#include "Route.h"
#include "Settings.h"

#include <vsg/commands/ClearAttachments.h>
#include <vsg/core/ref_ptr.h>

#include <string>

struct EditorParams;
class Signal;
class WindowHandler;

namespace vsg
{

class AmbientLight;
class Camera;
class Group;
class LookAt;
class OperationThreads;
class Options;
class Perspective;
class ShaderSet;
class Viewer;

}

class RouteEditor
{
public:
    RouteEditor();
    ~RouteEditor();

    bool initialize();
    void run();

private:
    void configure_shaders();

private:
    EditorState state = EditorState::SELECT_ROUTE;
    settings_t settings;
    vsg::ref_ptr<vsg::Options> options;
    vsg::ref_ptr<WindowHandler> window_handler;
    vsg::ref_ptr<MouseHandler> mouse_handler;
    vsg::ref_ptr<KeyboardHandler> keyboard_handler;
    vsg::ref_ptr<CameraHandler> camera_handler;
    vsg::ref_ptr<vsg::ClearAttachments> clear_attachments;
    vsg::ref_ptr<vsg::AmbientLight> ambient_light;
    vsg::ref_ptr<vsg::Group> scene_group;
    vsg::ref_ptr<vsg::Group> gui_group;
    vsg::ref_ptr<Route> route;
    vsg::ref_ptr<EditorParams> params;
    vsg::ref_ptr<vsg::Viewer> viewer;
    vsg::ref_ptr<ObjectSelector> object_selector;
};

#endif // ROUTE_EDITOR_H
