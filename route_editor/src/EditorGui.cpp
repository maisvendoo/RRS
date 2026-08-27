#include "EditorGui.h"

#include "Action.h"
#include "Camera.h"
#include "EditorContext.h"
#include "EditorState.h"
#include "Gizmo.h"
#include "Journal.h"
#include "KeyBindings.h"
#include "ObjectSelector.h"
#include "Route.h"
#include "RouteObject.h"
#include "SceneGraph.h"
#include "StateManager.h"
#include "commands/AddObject.h"
#include "commands/Command.h"
#include "commands/CommandList.h"
#include "commands/RotateObjects.h"
#include "commands/ScaleObjects.h"
#include "commands/TranslateObjects.h"
#include "filesystem.h"
#include "rail-signal.h"
#include "settings/CameraSettings.h"
#include "settings/GuiSettings.h"
#include "states/State.h"
#include "switch.h"
#include "topology.h"
#include "topology-defines.h"
#include "track.h"
#include "trajectory.h"
#include "vec3.h"

#include <ImGuiFileDialog.h>

#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/RecordTraversal.h>
#include <vsg/commands/Commands.h>
#include <vsg/core/Array.h>
#include <vsg/core/Data.h>
#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/common.h>
#include <vsg/maths/quat.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/ui/KeyEvent.h>

#include <vsgImGui/imgui.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <mutex>
#include <string>

#define SHOW_WINDOW(setting_name) if (gui_settings.setting_name) setting_name()

static bool drag_double(const char* label, double* data,
    const double* min = nullptr)
{
    return ImGui::DragScalar(label, ImGuiDataType_Double, data,
        1.0f, min, nullptr, "%.3f");
}

static bool drag_double3(const char* label, double* data, float speed = 1.0f,
    const double* min = nullptr, const double* max = nullptr,
    ImGuiSliderFlags flags = 0)
{
    return ImGui::DragScalarN(label, ImGuiDataType_Double, data, 3,
        speed, min, max, "%.3f", flags);
}

EditorGui::EditorGui(
    EditorContext& context,
    camera_settings_t& camera_settings,
    gui_settings_t& gui_settings,
    const KeyBindings& key_bindings,
    StateManager& state_manager,
    const vsg::ref_ptr<Camera>& camera,
    EditorState& editor_state,
    CommandList& command_list,
    const vsg::ref_ptr<Route>& route,
    std::string& route_dir,
    const vsg::ref_ptr<Gizmo>& gizmo
)
    : context_(context)
    , camera_settings(camera_settings)
    , gui_settings(gui_settings)
    , key_bindings(key_bindings)
    , state_manager(state_manager)
    , camera(camera)
    , editor_state(editor_state)
    , command_list(command_list)
    , route(route)
    , route_dir(route_dir)
    , gizmo(gizmo)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    add_ttf_font("JetBrainsMono-Regular.ttf",
        gui_settings.font_size, nullptr,
        io.Fonts->GetGlyphRangesCyrillic());

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    if (!gui_settings.is_editable)
    {
        window_flags_ |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    }

    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameBorderSize = 1.0f;
    style.ScrollbarSize = 16.0f;
    style.GrabMinSize = 16.0f;

    viewport = ImGui::GetMainViewport();
}

EditorGui::~EditorGui()
{
    // ImGui::DestroyContext();
}

void EditorGui::record(vsg::CommandBuffer& command_buffer) const
{
    (void)command_buffer;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New route"))
            {
                // TODO
            }

            if (ImGui::MenuItem("Load route"))
            {
                IGFD::FileDialogConfig config;
                config.path = FileSystem::getInstance().getRouteRootDir();
                ImGuiFileDialog::Instance()->OpenDialog("LoadRouteKey",
                    "Load route", nullptr, config);
            }

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    draw_status_bar();
    draw_load_route_file_dialog();
    draw_invalid_route_popup();
    state_manager.get_editor_state()->draw_gui();

    switch (editor_state)
    {
        case EditorState::SELECT_ROUTE:
        {
            return;
        }
        default:
        {
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::Begin("Settings", nullptr, window_flags_ | ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Checkbox("Show objects.ref", &gui_settings.show_objects_ref);
            ImGui::Checkbox("Show route1.map", &gui_settings.show_route_map);
            ImGui::Checkbox("Show stations", &gui_settings.show_stations_conf);
            ImGui::Checkbox("Show waypoints", &gui_settings.show_waypoints_conf);
            ImGui::Checkbox("Show key bindings", &gui_settings.show_key_bindings);
            ImGui::Checkbox("Show camera settings", &gui_settings.show_camera_settings);
            ImGui::Checkbox("Show topology", &gui_settings.show_topology);
            ImGui::Checkbox("Show selected objects properties", &gui_settings.show_selected_objects_properties);
            ImGui::Checkbox("Show commands", &gui_settings.show_commands);
            ImGui::End();

            ImGui::ShowDemoWindow();

            SHOW_WINDOW(show_objects_ref);
            SHOW_WINDOW(show_route_map);
            SHOW_WINDOW(show_stations_conf);
            SHOW_WINDOW(show_waypoints_conf);
            SHOW_WINDOW(show_key_bindings);
            SHOW_WINDOW(show_camera_settings);
            SHOW_WINDOW(show_topology);
            SHOW_WINDOW(show_selected_objects_properties);
            SHOW_WINDOW(show_commands);

            ImGui::Begin("TestProgressBars");

            float fraction = 1.0f;
            if (context_.total_static_objects_count != 0)
            {
                fraction = (float)context_.static_objects_count /
                    context_.total_static_objects_count;
            }

            char overlay[64];
            snprintf(overlay, 64, "%zu / %zu",
                context_.static_objects_count.load(),
                context_.total_static_objects_count.load());

            ImGui::ProgressBar(fraction, {200.0f, 30.0f}, overlay);

            if (context_.topology_loaded)
            {
                fraction = 1.0f;
                if (context_.total_topology_objects_count != 0)
                {
                    fraction = (float)context_.topology_objects_count /
                        context_.total_topology_objects_count;
                }

                snprintf(overlay, 64, "%zu / %zu",
                    context_.topology_objects_count.load(),
                    context_.total_topology_objects_count.load());

                ImGui::ProgressBar(fraction, {200.0f, 30.0f}, overlay);
            }
            else
            {
                ImGui::Text("Topology not yet loaded");
            }

            ImGui::End();

            return;
        }
    }
}

void EditorGui::show_objects_ref() const
{
    ImGui::Begin("objects_ref", nullptr, window_flags_);

    if (!route)
    {
        ImGui::Text("There is no route yet");
        ImGui::End();
        return;
    }

    static char search_buffer[256] = "";
    ImGui::InputTextWithHint("search_label", "", search_buffer, 256);

    std::string search_lower = search_buffer;
    std::transform(search_lower.begin(), search_lower.end(),
        search_lower.begin(), ::tolower);

    if (ImGui::BeginTable("objects_ref_table", 2,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg))
    {
        for (const auto& [label, ref] : context_.objects_ref)
        {
            std::string label_lower = label;
            std::transform(label_lower.begin(), label_lower.end(),
                label_lower.begin(), ::tolower);

            if (search_buffer[0] != '\0' &&
                label_lower.find(search_lower) == std::string::npos)
            {
                continue;
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Button(label.c_str()))
            {
                add_object(ref.paged_lod, label);
            }

            ImGui::TableNextColumn();
            ImGui::Text("%s", ref.relative_path.c_str());
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void EditorGui::show_route_map() const
{
    ImGui::Begin("route1.map", nullptr, window_flags_);

    if (!route)
    {
        ImGui::Text("There is no route yet");
        ImGui::End();
        return;
    }

    if (ImGui::BeginTable("route_map_table", 7,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg))
    {
        for (const auto& [label, transforms] : context_.route_map)
        {
            for (const auto& transform : transforms)
            {
                const vsg::dvec3& translation = transform.translation;
                const vsg::dvec3& rotation_deg = transform.rotation_deg;

                constexpr const char* number_format = "%10.3f";

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", label.c_str());
                ImGui::TableNextColumn();
                ImGui::Text(number_format, translation.x);
                ImGui::TableNextColumn();
                ImGui::Text(number_format, translation.y);
                ImGui::TableNextColumn();
                ImGui::Text(number_format, translation.z);
                ImGui::TableNextColumn();
                ImGui::Text(number_format, rotation_deg.x);
                ImGui::TableNextColumn();
                ImGui::Text(number_format, rotation_deg.y);
                ImGui::TableNextColumn();
                ImGui::Text(number_format, rotation_deg.z);
            }
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void EditorGui::show_stations_conf() const
{
    ImGui::Begin("stations.conf", nullptr, window_flags_);

    if (ImGui::BeginTable("stations_conf_table", 4,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg))
    {
        for (const auto& [label, translation] : context_.stations_conf)
        {
            constexpr const char* number_format = "%10.3f";

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Button(label.c_str()))
            {
                camera->get_look_at()->eye = translation +
                    vsg::dvec3(0.0, 0.0, 50.0);

                camera->get_look_at()->center =
                    camera->get_look_at()->eye +
                    camera->get_front();
            }
            ImGui::TableNextColumn();
            ImGui::Text(number_format, translation.x);
            ImGui::TableNextColumn();
            ImGui::Text(number_format, translation.y);
            ImGui::TableNextColumn();
            ImGui::Text(number_format, translation.z);
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

// TODO: Сделать, чтобы реальные позиции грузились один раз?
void EditorGui::show_waypoints_conf() const
{
    if (!context_.topology_loaded)
    {
        return;
    }

    ImGui::Begin("waypoints.conf", nullptr, window_flags_);

    if (ImGui::BeginTable("waypoints_conf_table", 5,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg))
    {
        for (const auto& [label, data] : context_.waypoints_conf)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Button(label.c_str()))
            {
                const traj_list_t* const traj_list =
                    context_.topology->getTrajectoriesList();

                const QString traj_name = QString::fromStdString(
                    data.trajectory_name);

                auto found_it = traj_list->find(traj_name);
                if (found_it == traj_list->end())
                {
                    Journal::instance()->error(QString("Failed to find trajectory %1")
                        .arg(data.trajectory_name.c_str()));
                }
                else
                {
                    Trajectory* const trajectory = *found_it;

                    auto pd = trajectory->getPosition(data.coord, data.direction);

                    const dvec3 pos = pd.position;

                    double h = 5.0;

                    camera->get_look_at()->eye =
                        vsg::dvec3(pos.x + pd.up.x * h,
                            pos.y + pd.up.y * h,
                            pos.z + pd.up.z * h);

                    camera->get_look_at()->center =
                        camera->get_look_at()->eye +
                        camera->get_front();
                }
            }
            ImGui::TableNextColumn();
            ImGui::Text("%s", data.trajectory_name.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%d", data.direction);
            ImGui::TableNextColumn();
            ImGui::Text("%10.3f", data.coord);
            ImGui::TableNextColumn();
            ImGui::Text("%10.3f", data.length);
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void EditorGui::show_key_bindings() const
{
    ImGui::Begin("Key Bindings", nullptr, window_flags_);

    if (ImGui::BeginTable("key_bindings_table", 2,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg))
    {
        for (int i = 0; i < TOTAL_ACTIONS; ++i)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", to_c_string(static_cast<Action>(i)));
            ImGui::TableNextColumn();

            std::string label;

            static const std::map<vsg::KeyModifier, const char*> test_map = {
                {vsg::MODKEY_Shift, "Shift"},
                {vsg::MODKEY_Control, "Ctrl"},
                {vsg::MODKEY_Alt, "Alt"}
            };

            for (const auto& [modifier, name] : test_map)
            {
                if (key_bindings.modifiers[i] & modifier)
                {
                    label += name;
                    label += " + ";
                }
            }

            label += std::toupper(key_bindings.keys[i]);
            ImGui::Text("%s", label.c_str());
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void EditorGui::show_camera_settings() const
{
    ImGui::Begin("Camera Settings", nullptr, window_flags_);

    constexpr double min = 0.0;

    ImGui::Text("Move speed:");
    drag_double("##move_speed", &camera_settings.move_speed, &min);

    ImGui::Text("Rotate speed:");
    drag_double("##rotate_speed", &camera_settings.rotate_speed, &min);

    ImGui::Text("Zoom power:");
    drag_double("##zoom_power", &camera_settings.zoom_power, &min);

    ImGui::Text("FovY:");

    if (ImGui::SliderScalar("##fovy", ImGuiDataType_Double, &camera_settings.fovy,
        &camera_settings.fovy_min, &camera_settings.fovy_max, "%.3f"))
    {
        camera->get_perspective()->fieldOfViewY = camera_settings.fovy;
    }

    ImGui::End();
}

void EditorGui::show_topology() const
{
    ImGui::Begin("Topology", nullptr, window_flags_);

    if (!route)
    {
        ImGui::Text("There is no route yet");
        ImGui::End();
        return;
    }

    std::lock_guard<std::mutex> lock_guard(context_.topology_mutex);
    if (!context_.topology)
    {
        ImGui::Text("There is no topology yet");
        ImGui::End();
        return;
    }

    const auto route_name = context_.topology->getRouteName().toStdString();
    ImGui::Text("Route name: %s", route_name.c_str());

    if (ImGui::CollapsingHeader("Trajectories"))
    {
        const auto* trajectories = context_.topology->getTrajectoriesList();
        for (const Trajectory* trajectory : *trajectories)
        {
            if (ImGui::TreeNode(trajectory->getName().toStdString().c_str()))
            {
                const auto& tracks = trajectory->getTracks();
                const auto tracks_size = tracks.size();

                for (auto i = decltype(tracks_size){0}; i < tracks_size; ++i)
                {
                    const track_t& track = tracks[i];
                    const dvec3& begin_point = track.begin_point;
                    const dvec3& end_point = track.end_point;

                    std::string label = "[";
                    label += std::to_string(i);
                    label += "]##";
                    label += trajectory->getName().toStdString();
                    ImGui::SeparatorText(label.c_str());
                    ImGui::Text("         begin: %12.3f %12.3f %12.3f",
                        begin_point.x, begin_point.y, begin_point.z);
                    ImGui::Text("           end: %12.3f %12.3f %12.3f",
                        end_point.x, end_point.y, end_point.z);
                    ImGui::Text("railway_coords: %12.3f %12.3f",
                        track.railway_coord0, track.railway_coord1);
                    ImGui::Text("    traj_coord: %12.3f", track.traj_coord);
                }

                ImGui::TreePop();
            }
        }
    }

    if (ImGui::CollapsingHeader("Switches"))
    {
        const auto print_traj = [](const char* type,
            const Trajectory* trajectory) -> void
        {
            ImGui::Text("%s: %s", type, trajectory
                ? trajectory->getName().toStdString().c_str()
                : "nullptr");
        };

        const auto print_signal = [](const char* type,
            const Signal* signal) -> void
        {
            if (signal)
            {
                ImGui::Text("SignalLiter%s: %s", type,
                    signal->getLetter().toStdString().c_str());

                ImGui::Text("SignalModel%s: %s", type,
                    signal->getSignalModel().toStdString().c_str());

                const dvec3& rel_pos = signal->getRelPos();
                const dvec3& rel_rot = signal->getRelRot();

                ImGui::Text("RelPos%s: %8.3f %8.3f %8.3f", type,
                    rel_pos.x, rel_pos.y, rel_pos.z);

                ImGui::Text("RelRot%s: %8.3f %8.3f %8.3f", type,
                    rel_rot.x, rel_rot.y, rel_rot.z);
            }
        };

        const sw_list_t* const connectors = context_.topology->getConnectorsList();
        for (auto it = connectors->constBegin(); it != connectors->constEnd(); ++it)
        {
            const Switch* const switch_ = dynamic_cast<Switch*>(*it);
            if (!switch_)
            {
                continue;
            }

            if (ImGui::TreeNode(switch_->getName().toStdString().c_str()))
            {
                print_traj("bwdMinusTraj", switch_->get_bwd_minus_traj());
                print_traj("bwdPlusTraj", switch_->get_bwd_plus_traj());
                print_traj("fwdMinusTraj", switch_->get_fwd_minus_traj());
                print_traj("fwdPlusTraj", switch_->get_fwd_plus_traj());

                print_signal("Bwd", switch_->getSignalBwd());
                print_signal("Fwd", switch_->getSignalFwd());

                ImGui::TreePop();
            }
        }
    }

    ImGui::End();
}

void EditorGui::show_selected_objects_properties() const
{
    if (!context_.object_selector)
    {
        return;
    }

    const auto& selected_objects = context_.selected_objects;
    if (selected_objects.empty())
    {
        return;
    }

    ImGui::Begin("Selected objects", nullptr, window_flags_);

    static bool dragging = false;

    std::size_t i = 0;

    for (const auto& object : selected_objects)
    {
        ImGui::Text("label: %s", object->label.c_str());

        handle_translation_drag(i, object, dragging);
        handle_rotation_drag(i, object, dragging);
        handle_scale_drag(i, object, dragging);

        ++i;
    }

    ImGui::End();
}

void EditorGui::show_commands() const
{
    ImGui::Begin("Commands");
    auto active = command_list.get_active();
    auto curr = command_list.get_tail();
    while (curr)
    {
        if (curr == active)
        {
            ImGui::TextColored(ImVec4{0.2f, 1.0f, 0.3f, 1.0f}, "%s",
                curr->command->get_description());
            ImGui::Separator();
        }
        else
        {
            ImGui::Text("%s", curr->command->get_description());
            ImGui::Separator();
        }

        curr = curr->prev;
    }
    ImGui::End();
}

void EditorGui::add_object(
    const vsg::ref_ptr<vsg::PagedLOD>& paged_lod,
    const std::string& label
) const
{
    const auto object = RouteObject::create(context_, paged_lod, gizmo, label,
        camera->get_look_at()->eye +
        camera->get_front() * 20.0);

    command_list.push(new AddObject(context_, object, route, gizmo), true);
}

void EditorGui::save_objects_matrixes() const
{
    for (const auto& object : context_.selected_objects)
    {
        object->save_matrix();
    }
}

void EditorGui::handle_translation_drag(
    size_t index,
    const vsg::ref_ptr<RouteObject>& object,
    bool& dragging
) const
{
    std::string label = "translation##" + std::to_string(index);
    static vsg::dvec3 total_translation = {0.0, 0.0, 0.0};

    vsg::dvec3 translation = object->get_translation();
    if (drag_double3(label.c_str(), translation.data()))
    {
        if (!dragging)
        {
            total_translation = {0.0, 0.0, 0.0};
            save_objects_matrixes();
            dragging = true;
        }
        total_translation += translation - object->get_translation();
        object->set_translation(translation);
    }

    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        command_list.push(new TranslateObjects(context_, {object},
            total_translation), false);
        dragging = false;
    }
}

void EditorGui::handle_rotation_drag(
    std::size_t index,
    const vsg::ref_ptr<RouteObject>& object,
    bool& dragging
) const
{
    std::string label = "rotation##" + std::to_string(index);
    static vsg::dvec3 total_rotation_deg = {0.0, 0.0, 0.0};

    constexpr double min_rot_deg = -360.0;
    constexpr double max_rot_deg = 360.0;
    vsg::dvec3 rotation_deg = object->get_rotation_deg();
    if (drag_double3(label.c_str(), rotation_deg.data(), 1.0f,
        &min_rot_deg, &max_rot_deg, ImGuiSliderFlags_WrapAround))
    {
        if (!dragging)
        {
            total_rotation_deg = {0.0, 0.0, 0.0};
            save_objects_matrixes();
            dragging = true;
        }
        total_rotation_deg += rotation_deg - object->get_rotation_deg();
        object->set_rotation_deg(rotation_deg);
    }

    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        vsg::dvec3 axis = {0.0, 0.0, 0.0};
        double radians;

        if (std::abs(total_rotation_deg.x) >= 1.0e-6)
        {
            axis.x = 1.0;
            radians = vsg::radians(total_rotation_deg.x);
        }
        else if (std::abs(total_rotation_deg.y) >= 1.0e-6)
        {
            axis.y = 1.0;
            radians = vsg::radians(total_rotation_deg.y);
        }
        else
        {
            axis.z = 1.0;
            radians = vsg::radians(total_rotation_deg.z);
        }

        command_list.push(new RotateObjects(context_, {object},
            gizmo->get_curr_pos(), axis, radians), false);
        dragging = false;
    }
}

void EditorGui::handle_scale_drag(
    size_t index,
    const vsg::ref_ptr<RouteObject>& object,
    bool& dragging
) const
{
    std::string label = "scale##" + std::to_string(index);
    static vsg::dvec3 total_scale = {1.0, 1.0, 1.0};

    const vsg::dvec3& prev_scale = object->get_scale();
    vsg::dvec3 scale = object->get_scale();
    if (drag_double3(label.c_str(), scale.data(), 0.01f))
    {
        if (vsg::length(scale) > 1.0e-6)
        {
            if (!dragging)
            {
                total_scale = {1.0, 1.0, 1.0};
                save_objects_matrixes();
                dragging = true;
            }
            total_scale *= {scale.x / prev_scale.x, scale.y / prev_scale.y,
                scale.z / prev_scale.z};
            object->set_scale(scale);
        }
    }

    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        command_list.push(new ScaleObjects(context_, {object},
            gizmo->get_curr_pos(), total_scale), false);
        dragging = false;
    }
}

void EditorGui::add_ttf_font(
    const char* filename,
    float size_pixels,
    const ImFontConfig* font_cfg,
    const ImWchar* glyph_ranges
)
{
    ImGuiIO& io = ImGui::GetIO();
    const FileSystem& fs = FileSystem::getInstance();
    const std::string font_path = fs.combinePath(fs.getFontsDir(), filename);
    io.Fonts->AddFontFromFileTTF(font_path.c_str(), size_pixels, font_cfg,
        glyph_ranges);
}

void EditorGui::draw_status_bar() const
{
    ImGui::SetNextWindowPos(ImVec2(
        viewport->Pos.x,
        viewport->Pos.y + viewport->Size.y - ImGui::GetFrameHeight() * 1.5
    ));
    ImGui::SetNextWindowSize(ImVec2(
        viewport->Size.x,
        ImGui::GetFrameHeight() * 1.5
    ));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize;
    // ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
    // ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse |
    // ImGuiWindowFlags_NoSavedSettings |
    // ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground;
    // ImGuiWindowFlags_MenuBar;

    if (ImGui::Begin("StatusBar", nullptr, flags))
    {
        state_manager.get_editor_state()->fill_status_bar();
        ImGui::End();
    }
}

void EditorGui::draw_load_route_file_dialog() const
{
    if (ImGuiFileDialog::Instance()->IsOpened("LoadRouteKey"))
    {
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(ImVec2(
            viewport->WorkSize.x,
            viewport->WorkSize.y - ImGui::GetFrameHeight() * 1.5
        ));
    }

    if (ImGuiFileDialog::Instance()->Display("LoadRouteKey",
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            route_dir = ImGuiFileDialog::Instance()->GetCurrentPath();
            if (!std::filesystem::exists(route_dir + "/models") ||
                !std::filesystem::exists(route_dir + "/textures") ||
                !std::filesystem::exists(route_dir + "/topology") ||
                !std::filesystem::exists(route_dir + "/objects.ref"))
            {
                ImGui::OpenPopup("InvalidRoute");
            }
            else
            {
                state_manager.defer_switch_to(STATE_BASIC);
                editor_state = EditorState::LOAD_ROUTE;
                ImGuiFileDialog::Instance()->Close();
            }
        }
        else
        {
            ImGuiFileDialog::Instance()->Close();
        }
    }
}

void EditorGui::draw_invalid_route_popup() const
{
    if (ImGui::BeginPopupModal("InvalidRoute", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
    {
        ImGui::Text(
            "Invalid route!\n"
            "Route must contain:\n"
            "models/\n"
            "textures/\n"
            "topology/\n"
            "objects.ref"
        );

        if (ImGui::Button("OK", ImVec2(-FLT_MIN, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
