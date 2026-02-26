#ifndef ROUTE_EDITOR_H
#define ROUTE_EDITOR_H

#include "CommandList.h"
#include "EditorContext.h"
#include "EditorState.h"
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
class Options;
class RenderGraph;
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
    EditorContext context;
};

#endif // ROUTE_EDITOR_H
