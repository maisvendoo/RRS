#include "EditorContext.h"

#include "ObjectSelector.h"
#include "WindowHandler.h"
#include "MouseHandler.h"
#include "KeyboardHandler.h"
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/Camera.h>
#include <vsg/app/RenderGraph.h>
#include <vsg/app/Viewer.h>
#include <vsg/commands/ClearAttachments.h>
#include "CameraHandler.h"
#include "IntersectionHandler.h"
#include "SceneGraph.h"
#include "topology.h"

EditorContext::~EditorContext()
{
    delete topology;
}
