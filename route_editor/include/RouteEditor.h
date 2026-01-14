#ifndef ROUTE_EDITOR_H
#define ROUTE_EDITOR_H

#include "EditorState.h"
#include "Settings.h"

#include <vsg/core/ref_ptr.h>

class CameraHandler;
struct EditorParams;
class KeyboardHandler;
class MouseHandler;
class ObjectSelector;
class SceneGraph;
class Signal;
class WindowHandler;

namespace vsg
{

class ClearAttachments;
class Group;
class Options;
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
    vsg::ref_ptr<SceneGraph> scene_graph;
    vsg::ref_ptr<vsg::Group> gui_group;
    vsg::ref_ptr<EditorParams> params;
    vsg::ref_ptr<vsg::Viewer> viewer;
    vsg::ref_ptr<ObjectSelector> object_selector;
};

#endif // ROUTE_EDITOR_H
