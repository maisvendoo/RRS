#include "EditorContext.h"

#include "Gizmo.h"
#include "IntersectionHandler.h"
#include "ObjectSelector.h"
#include "Outline.h"
#include "Route.h"
#include "SceneGraph.h"
#include "topology.h"
#include "WindowHandler.h"

#include <vsg/commands/ClearAttachments.h>
#include <vsg/nodes/PagedLOD.h>

EditorContext::EditorContext() = default;
EditorContext::~EditorContext() = default;
