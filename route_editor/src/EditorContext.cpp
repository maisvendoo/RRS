#include "EditorContext.h"

#include "Camera.h"
#include "Gizmo.h"
#include "IntersectionHandler.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ObjectSelector.h"
#include "Outline.h"
#include "Route.h"
#include "SceneGraph.h"
#include "topology.h"
#include "WindowHandler.h"

#include <vsg/app/Camera.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/commands/ClearAttachments.h>
#include <vsg/nodes/PagedLOD.h>

EditorContext::EditorContext() = default;
EditorContext::~EditorContext() = default;
