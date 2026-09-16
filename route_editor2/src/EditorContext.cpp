#include "editor/EditorContext.h"

#include "editor/TrajectoryPicker.h"

#include "editor/Camera.h"
#include "editor/EventHandler.h"
#include "editor/IntersectionHandler.h"
#include "editor/MeasureTool.h"
#include "editor/ObjectSelector.h"
#include "editor/Route.h"
#include "editor/SplineTool.h"
#include "editor/settings/CameraSettings.h"
#include "editor/settings/GuiSettings.h"

#include <topology.h>

#include <vsg/nodes/Group.h>
#include <vsg/app/Viewer.h>

EditorContext::EditorContext(camera_settings_t& camera_settings_,
                             gui_settings_t& gui_settings_)
    : camera_settings(camera_settings_)
    , gui_settings(gui_settings_)
{
    // Инструмент измерения живёт вместе с контекстом
    measure_tool = std::make_unique<MeasureTool>(*this);

    // Инструмент построения новых путей - по образцу измерения
    spline_tool = std::make_unique<SplineTool>(*this);
}

EditorContext::~EditorContext()
{
    if (load_static_objects_thread.joinable())
    {
        load_static_objects_thread.join();
    }

    if (load_topology_thread.joinable())
    {
        load_topology_thread.join();
    }
}
