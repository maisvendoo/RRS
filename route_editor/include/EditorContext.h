#ifndef EDITOR_CONTEXT_H
#define EDITOR_CONTEXT_H

#include "CommandList.h"
#include "EditorState.h"
#include "RouteObject.h"
#include "Settings.h"

#include <vsg/core/ref_ptr.h>

#include <string>

class CameraHandler;
class IntersectionHandler;
class KeyboardHandler;
class MouseHandler;
class ObjectSelector;
class SceneGraph;
class WindowHandler;

namespace vsg
{

class ClearAttachments;
class LookAt;
class Options;
class Perspective;
class RenderGraph;
class Viewer;

}

struct EditorContext
{
    EditorState state = EditorState::SELECT_ROUTE;
    settings_t settings;
    CommandList commands;
    vsg::ref_ptr<vsg::Options> options;
    vsg::ref_ptr<WindowHandler> window_handler;
    vsg::ref_ptr<MouseHandler> mouse_handler;
    vsg::ref_ptr<KeyboardHandler> keyboard_handler;
    vsg::ref_ptr<vsg::Perspective> perspective;
    vsg::ref_ptr<vsg::LookAt> look_at;
    vsg::ref_ptr<CameraHandler> camera_handler;
    vsg::ref_ptr<IntersectionHandler> intersection_handler;
    vsg::ref_ptr<SceneGraph> scene_graph;
    vsg::ref_ptr<vsg::ClearAttachments> clear_attachments;
    vsg::ref_ptr<vsg::RenderGraph> render_graph;
    vsg::ref_ptr<vsg::Viewer> viewer;
    vsg::ref_ptr<ObjectSelector> object_selector;
    RouteObjects selected_objects;
    RouteObjects hidden_objects;
    std::string route_dir;
};

#endif // EDITOR_CONTEXT_H
