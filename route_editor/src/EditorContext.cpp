#include "EditorContext.h"

#include "CameraHandler.h"
#include "Gizmo.h"
#include "IntersectionHandler.h"
#include "KeyboardHandler.h"
#include "MouseHandler.h"
#include "ObjectSelector.h"
#include "Outline.h"
#include "Route.h"
#include "SceneGraph.h"
#include "WindowHandler.h"
#include "topology.h"

#include <vsg/app/Camera.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/RenderGraph.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/app/Viewer.h>
#include <vsg/app/Window.h>
#include <vsg/commands/ClearAttachments.h>
#include <vsg/io/Options.h>

EditorContext::EditorContext() = default;

EditorContext::~EditorContext()
{
    delete topology;
}
