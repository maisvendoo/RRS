#include "editor/EditorContext.h"

#include "editor/Camera.h"
#include "editor/Gizmo.h"
#include "editor/Keyboard.h"
#include "editor/Mouse.h"
#include "editor/ObjectManager.h"
#include "editor/ObjectSelector.h"
#include "editor/Outline.h"
#include "editor/Route.h"
#include "editor/RouteObject.h"
#include "editor/StateManager.h"
#include "editor/WorldCulling.h"
#include "editor/commands/CommandManager.h"

#include <topology.h>

#include <vsg/app/Window.h>
#include <vsg/commands/ClearAttachments.h>
#include <vsg/nodes/PagedLOD.h>

EditorContext::EditorContext() = default;

EditorContext::~EditorContext() = default;
