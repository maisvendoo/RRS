#include "editor/EditorGui.h"

#include "editor/Action.h"
#include "editor/Camera.h"
#include "editor/EditorContext.h"
#include "editor/EditorState.h"
#include "editor/Gizmo.h"
#include "editor/KeyBindings.h"
#include "editor/ObjectSelector.h"
#include "editor/Route.h"
#include "editor/RouteObject.h"
#include "editor/StateManager.h"
#include "editor/commands/AddObjectCommand.h"
#include "editor/commands/Command.h"
#include "editor/commands/CommandManager.h"
#include "editor/commands/RotateObjectsCommand.h"
#include "editor/commands/ScaleObjectsCommand.h"
#include "editor/commands/TranslateObjectsCommand.h"
#include "editor/gui/CameraSettingsWindow.h"
#include "editor/settings/GuiSettings.h"
#include "editor/states/State.h"

#include <Journal.h>
#include <filesystem.h>
#include <rail-signal.h>
#include <switch.h>
#include <topology-defines.h>
#include <topology.h>
#include <track.h>
#include <trajectory.h>
#include <vec3.h>

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
#include <functional>
#include <memory>
#include <string>

#define DECOMPOSE_VEC2(vec) vec.x, vec.y
#define DECOMPOSE_VEC3(vec) vec.x, vec.y, vec.z

static bool drag_double3(const char* label, double* data, float speed = 1.0f,
                         const double* min = nullptr,
                         const double* max = nullptr,
                         ImGuiSliderFlags flags = 0)
{
    return ImGui::DragScalarN(label, ImGuiDataType_Double, data, 3, speed, min,
                              max, "%.3f", flags);
}

EditorGui::EditorGui(EditorContext& context)
    : editor_ctx(context)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // io.IniFilename = nullptr;

    const auto& gui_settings = context.gui_settings;

    add_ttf_font("JetBrainsMono-Regular.ttf", gui_settings.font_size, nullptr,
                 io.Fonts->GetGlyphRangesCyrillic());

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    if (!gui_settings.is_editable)
    {
        window_flags_ |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    }

    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameBorderSize = 1.0f;
    // style.FrameRounding = 3.0f;
    // style.WindowRounding = 3.0f;
    style.ScrollbarSize = 16.0f;
    style.GrabMinSize = 16.0f;

    viewport = ImGui::GetMainViewport();

    camera_settings_window =
        std::make_unique<CameraSettingsWindow>(editor_ctx, window_flags_);
}

EditorGui::~EditorGui()
{
    // ImGui::DestroyContext();
}

void EditorGui::record(
    [[maybe_unused]] vsg::CommandBuffer& command_buffer) const
{
    // ImGui::DockSpaceOverViewport(0, viewport);

    auto& gui_settings = editor_ctx.gui_settings;

    draw_main_menu_bar();
    draw_status_bar();
    draw_invalid_route_popup();

    editor_ctx.state_manager->get_curr_state()->draw_gui();

    switch (editor_ctx.editor_state)
    {
    case EditorState::SELECT_ROUTE:
        return;
    default:
        // ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::Begin("Settings", nullptr, window_flags_);
        ImGui::Checkbox("Show objects.ref", &gui_settings.show_objects_ref);
        ImGui::Checkbox("Show route1.map", &gui_settings.show_route_map);
        ImGui::Checkbox("Show stations", &gui_settings.show_stations_conf);
        ImGui::Checkbox("Show waypoints", &gui_settings.show_waypoints_conf);
        ImGui::Checkbox("Show key bindings", &gui_settings.show_key_bindings);
        ImGui::Checkbox("Show camera settings",
                        &gui_settings.show_camera_settings);
        ImGui::Checkbox("Show topology", &gui_settings.show_topology);
        ImGui::Checkbox("Show selected objects properties",
                        &gui_settings.show_selected_objects_properties);
        ImGui::Checkbox("Show commands", &gui_settings.show_commands);
        ImGui::End();

        // ImGui::ShowDemoWindow();

        show_objects_ref();
        show_route_map();
        show_stations_conf();
        show_waypoints_conf();
        show_key_bindings();
        camera_settings_window->show();
        show_topology();
        show_selected_objects_properties();
        show_commands();

        return;
    }
}

void EditorGui::show_objects_ref() const
{
    bool& show_objects_ref = editor_ctx.gui_settings.show_objects_ref;
    if (!show_objects_ref)
    {
        return;
    }

    ImGui::Begin("objects_ref", &show_objects_ref, window_flags_);

    static char search_buffer[256] = "";
    ImGui::InputText("label", search_buffer, 256);

    std::string search_lower = search_buffer;
    std::transform(search_lower.begin(), search_lower.end(),
                   search_lower.begin(), ::tolower);

    if (ImGui::BeginTable("objects_ref_table", 2,
                          ImGuiTableFlags_SizingFixedFit |
                          ImGuiTableFlags_Borders |
                          ImGuiTableFlags_RowBg))
    {
        for (const auto& [label, ref] : editor_ctx.objects_ref)
        {
            std::string label_lower = label;
            std::transform(label_lower.begin(), label_lower.end(),
                           label_lower.begin(), ::tolower);

            if (search_buffer[0] != '\0'
                && label_lower.find(search_lower) == std::string::npos)
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
    bool& show_route_map = editor_ctx.gui_settings.show_route_map;
    const auto& route_map = editor_ctx.route_map;

    if (!show_route_map)
    {
        return;
    }

    ImGui::Begin("route1.map", &show_route_map, window_flags_);

    if (ImGui::BeginTable("route_map_table", 7,
                          ImGuiTableFlags_SizingFixedFit |
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        for (const auto& [label, transforms] : route_map)
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
    const auto& camera = editor_ctx.camera;
    bool& show_stations_conf = editor_ctx.gui_settings.show_stations_conf;
    const auto& stations_conf = editor_ctx.stations_conf;

    if (!show_stations_conf)
    {
        return;
    }

    ImGui::Begin("stations.conf", &show_stations_conf, window_flags_);

    static char search_buffer[256] = "";
    ImGui::InputText("label", search_buffer, 256);

    std::string search_lower = search_buffer;
    std::transform(search_lower.begin(), search_lower.end(),
                   search_lower.begin(), ::tolower);

    if (ImGui::BeginTable("stations_conf_table", 4,
                          ImGuiTableFlags_SizingFixedFit |
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        for (const auto& [label, translation] : stations_conf)
        {
            std::string label_lower = label;
            std::transform(label_lower.begin(), label_lower.end(),
                           label_lower.begin(), ::tolower);

            if (search_buffer[0] != '\0' &&
                label_lower.find(search_lower) == std::string::npos)
            {
                continue;
            }

            constexpr const char* number_format = "%10.3f";

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            if (ImGui::Button(label.c_str()))
            {
                camera->look_on(translation);
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
    const auto& camera = editor_ctx.camera;
    bool& show_waypoints_conf = editor_ctx.gui_settings.show_waypoints_conf;
    const auto& topology_loaded = editor_ctx.topology_loaded;
    const auto& waypoints_conf = editor_ctx.waypoints_conf;

    if (!show_waypoints_conf || !topology_loaded.load())
    {
        return;
    }

    const auto topology_guard = editor_ctx.topology.lock();
    const auto& topology = *topology_guard;

    ImGui::Begin("waypoints.conf", &show_waypoints_conf, window_flags_);

    static char search_buffer[256] = "";
    ImGui::InputText("label", search_buffer, 256);

    std::string search_lower = search_buffer;
    std::transform(search_lower.begin(), search_lower.end(),
                   search_lower.begin(), ::tolower);

    if (ImGui::BeginTable("waypoints_conf_table", 5,
                          ImGuiTableFlags_SizingFixedFit
                              | ImGuiTableFlags_Borders
                              | ImGuiTableFlags_RowBg))
    {
        for (const auto& [label, data] : waypoints_conf)
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

            const QString traj_name =
                QString::fromStdString(data.trajectory_name);

            if (ImGui::Button(label.c_str()))
            {
                const traj_list_t* const traj_list =
                    topology->getTrajectoriesList();

                const auto found_it = traj_list->find(traj_name);
                if (found_it == traj_list->cend())
                {
                    Journal::instance()->error(
                        QString("Failed to find trajectory %1").arg(traj_name));
                    return;
                }

                const Trajectory* const trajectory = *found_it;
                const auto traj_pos =
                    trajectory->getPosition(data.coord, data.direction);
                const dvec3 pos = traj_pos.position;
                camera->look_on(vsg::dvec3(DECOMPOSE_VEC3(pos)));
            }

            ImGui::TableNextColumn();
            ImGui::Text("%s", traj_name.toStdString().c_str());
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
    bool& show_key_bindings = editor_ctx.gui_settings.show_key_bindings;
    const auto& key_bindings = editor_ctx.key_bindings;

    if (!show_key_bindings)
    {
        return;
    }

    ImGui::Begin("Key Bindings", &show_key_bindings, window_flags_);

    if (ImGui::BeginTable("key_bindings_table", 2,
                          ImGuiTableFlags_SizingFixedFit
                              | ImGuiTableFlags_Borders
                              | ImGuiTableFlags_RowBg))
    {
        for (int i = 0; i < TOTAL_ACTIONS; ++i)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", to_string(static_cast<Action>(i)).c_str());
            ImGui::TableNextColumn();

            std::string label;

            static const std::map<vsg::KeyModifier, std::string> test_map
                = {{vsg::MODKEY_Shift, "Shift"},
                   {vsg::MODKEY_Control, "Ctrl"},
                   {vsg::MODKEY_Alt, "Alt"}};

            for (const auto& [modifier, name] : test_map)
            {
                if (key_bindings.modifiers[i] & modifier)
                {
                    label += name + " + ";
                }
            }

            label += std::toupper(key_bindings.keys[i]);
            ImGui::Text("%s", label.c_str());
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void EditorGui::show_topology() const
{
    bool& show_topology = editor_ctx.gui_settings.show_topology;

    if (!show_topology)
    {
        return;
    }

    ImGui::Begin("Topology", &show_topology, window_flags_);

    auto topology_guard = editor_ctx.topology.lock();
    auto& topology = *topology_guard;

    if (!topology)
    {
        ImGui::Text("Topology not yet loaded");
        ImGui::End();
        return;
    }

    const auto route_name = topology->getRouteName().toStdString();
    ImGui::Text("Route name: %s", route_name.c_str());

    if (ImGui::CollapsingHeader("Trajectories"))
    {
        static char search_buffer[256] = "";
        ImGui::InputText("traj_name", search_buffer, 256);

        std::string search_lower = search_buffer;
        std::transform(search_lower.begin(), search_lower.end(),
                       search_lower.begin(), ::tolower);

        const auto* trajectories = topology->getTrajectoriesList();
        for (const Trajectory* trajectory : *trajectories)
        {
            const std::string trajectory_name
                = trajectory->getName().toStdString();
            std::string label_lower = trajectory_name;
            std::transform(label_lower.begin(), label_lower.end(),
                           label_lower.begin(), ::tolower);

            if (search_buffer[0] != '\0'
                && label_lower.find(search_lower) == std::string::npos)
            {
                continue;
            }

            if (ImGui::TreeNode(trajectory_name.c_str()))
            {
                const auto& tracks = trajectory->getTracks();
                const auto tracks_size = tracks.size();

                for (auto i = decltype(tracks_size){0}; i < tracks_size; ++i)
                {
                    const track_t& track = tracks[i];
                    const dvec3& p1 = track.begin_point;
                    const dvec3& p2 = track.end_point;

                    if (i == 0)
                    {
                        std::string label = "Jump##" + trajectory_name;
                        if (ImGui::Button(label.c_str()))
                        {
                            editor_ctx.camera->look_on(
                                vsg::dvec3(DECOMPOSE_VEC3(p1)));
                        }
                    }

                    constexpr const char* const float_format = "%12.3f";
                    constexpr std::size_t format_size = 64;
                    constexpr int alignment = 14;
                    char outer_format1[format_size];
                    char outer_format2[format_size];
                    char format[format_size];

                    std::snprintf(outer_format1, format_size, "%%%ds: %%s",
                                  alignment);

                    std::string label
                        = "[" + std::to_string(i) + "]##" + trajectory_name;
                    ImGui::SeparatorText(label.c_str());

                    std::snprintf(outer_format2, format_size, outer_format1,
                                  "begin", "%s %s %s");
                    std::snprintf(format, format_size, outer_format2,
                                  float_format, float_format, float_format);
                    ImGui::Text(format, DECOMPOSE_VEC3(p1));

                    std::snprintf(outer_format2, format_size, outer_format1,
                                  "end", "%s %s %s");
                    std::snprintf(format, format_size, outer_format2,
                                  float_format, float_format, float_format);
                    ImGui::Text(format, DECOMPOSE_VEC3(p2));

                    std::snprintf(outer_format2, format_size, outer_format1,
                                  "railway_coords", "%s %s");
                    std::snprintf(format, format_size, outer_format2,
                                  float_format, float_format);
                    ImGui::Text(format, track.railway_coord0,
                                track.railway_coord1);

                    std::snprintf(outer_format2, format_size, outer_format1,
                                  "traj_coord", "%s");
                    std::snprintf(format, format_size, outer_format2,
                                  float_format);
                    ImGui::Text(format, track.traj_coord);
                }

                ImGui::TreePop();
            }
        }
    }

    if (ImGui::CollapsingHeader("Switches"))
    {
        const auto print_traj
            = [](const char* type, const Trajectory* trajectory) -> void
        {
            ImGui::Text("%s: %s", type,
                        trajectory ? trajectory->getName().toStdString().c_str()
                                   : "nullptr");
        };

        const auto print_signal =
            [](const char* type, const Signal* signal) -> void
        {
            if (!signal)
            {
                return;
            }

            ImGui::Text("SignalLiter%s: %s", type,
                        signal->getLetter().toStdString().c_str());

            ImGui::Text("SignalModel%s: %s", type,
                        signal->getSignalModel().toStdString().c_str());

            const dvec3& rel_pos = signal->getRelPos();
            const dvec3& rel_rot = signal->getRelRot();

            ImGui::Text("RelPos%s: %8.3f %8.3f %8.3f", type,
                        DECOMPOSE_VEC3(rel_pos));

            ImGui::Text("RelRot%s: %8.3f %8.3f %8.3f", type,
                        DECOMPOSE_VEC3(rel_rot));
        };

        const sw_list_t* const connectors = topology->getConnectorsList();
        for (auto it = connectors->constBegin(); it != connectors->constEnd();
             ++it)
        {
            const Switch* const switch_ = dynamic_cast<const Switch*>(*it);
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
    bool& show_selected_objects_properties =
        editor_ctx.gui_settings.show_selected_objects_properties;

    if (!show_selected_objects_properties)
    {
        return;
    }

    if (!editor_ctx.object_selector)
    {
        return;
    }

    const auto& selected_objects = editor_ctx.selected_objects;
    if (selected_objects.empty())
    {
        return;
    }

    ImGui::Begin("Selected objects", &show_selected_objects_properties,
                 window_flags_);

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
    const auto& command_manager = editor_ctx.command_manager;
    bool& show_commands = editor_ctx.gui_settings.show_commands;

    if (!show_commands)
    {
        return;
    }

    ImGui::Begin("Commands", &show_commands, window_flags_);

    command_manager->for_each_command(
        [](const std::unique_ptr<::Command>& command) -> void
        {
            ImGui::Text("%s", command->get_description());
            ImGui::Separator();
        });

    command_manager->for_each_undone(
        [](const std::unique_ptr<::Command>& command) -> void
        {
            const ImVec4 text_color = {0.3f, 0.3f, 0.3f, 1.0f};
            ImGui::TextColored(text_color, "%s", command->get_description());
            ImGui::Separator();
        });

    ImGui::End();
}

void EditorGui::add_object(const vsg::ref_ptr<vsg::PagedLOD>& paged_lod,
                           const std::string& label) const
{
    const auto& camera = editor_ctx.camera;
    const auto& command_manager = editor_ctx.command_manager;

    const auto object = RouteObject::create(editor_ctx, paged_lod, label,
                                            camera->get_look_at()->eye
                                                + camera->get_front() * 20.0);

    auto command = std::make_unique<AddObjectCommand>(editor_ctx, object);
    command->execute();
    command_manager->push(std::move(command));
}

void EditorGui::save_objects_matrixes() const
{
    for (const auto& object : editor_ctx.selected_objects)
    {
        object->save_matrix();
    }
}

void EditorGui::handle_translation_drag(std::size_t index,
                                        const vsg::ref_ptr<RouteObject>& object,
                                        bool& dragging) const
{
    std::string label = "translation##" + std::to_string(index);
    static vsg::dvec3 total_translation = {0.0, 0.0, 0.0};

    vsg::dvec3 translation = object->get_position();
    if (drag_double3(label.c_str(), translation.data()))
    {
        if (!dragging)
        {
            total_translation = {0.0, 0.0, 0.0};
            save_objects_matrixes();
            dragging = true;
        }
        total_translation += translation - object->get_position();
        object->set_position(translation);
    }

    if (!ImGui::IsItemDeactivatedAfterEdit())
    {
        return;
    }

    auto command = std::make_unique<TranslateObjectsCommand>(
        editor_ctx, RouteObjects{object}, total_translation);
    editor_ctx.command_manager->push(std::move(command));

    dragging = false;
}

void EditorGui::handle_rotation_drag(std::size_t index,
                                     const vsg::ref_ptr<RouteObject>& object,
                                     bool& dragging) const
{
    std::string label = "rotation##" + std::to_string(index);
    static vsg::dvec3 total_rotation_deg = {0.0, 0.0, 0.0};

    constexpr double min_rot_deg = -360.0;
    constexpr double max_rot_deg = 360.0;
    vsg::dvec3 rotation_deg = object->get_rot_deg();
    if (drag_double3(label.c_str(), rotation_deg.data(), 1.0f, &min_rot_deg,
                     &max_rot_deg, ImGuiSliderFlags_WrapAround))
    {
        if (!dragging)
        {
            total_rotation_deg = {0.0, 0.0, 0.0};
            save_objects_matrixes();
            dragging = true;
        }
        total_rotation_deg += rotation_deg - object->get_rot_deg();
        object->set_rot_deg(rotation_deg);
    }

    if (!ImGui::IsItemDeactivatedAfterEdit())
    {
        return;
    }

    vsg::dvec3 axis = {0.0, 0.0, 0.0};
    double radians;

    for (int axis_index = 0; axis_index < 3; ++axis_index)
    {
        if (std::abs(total_rotation_deg[axis_index]) < 1.0e-6)
        {
            continue;
        }

        axis[axis_index] = 1.0;
        radians = vsg::radians(total_rotation_deg[axis_index]);
        break;
    }

    const auto& command_manager = editor_ctx.command_manager;
    const auto& gizmo = editor_ctx.gizmo;

    auto command = std::make_unique<RotateObjectsCommand>(
        editor_ctx, RouteObjects{object}, gizmo->get_curr_pos(), axis,
        radians);
    command_manager->push(std::move(command));

    dragging = false;
}

void EditorGui::handle_scale_drag(std::size_t index,
                                  const vsg::ref_ptr<RouteObject>& object,
                                  bool& dragging) const
{
    std::string label = "scale##" + std::to_string(index);
    static vsg::dvec3 total_scale = {1.0, 1.0, 1.0};

    const vsg::dvec3& prev_scale = object->get_scale();
    vsg::dvec3 scale = object->get_scale();
    if (drag_double3(label.c_str(), scale.data(), 0.01f))
    {
        if (vsg::length(scale) < 1.0e-6)
        {
            return;
        }

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

    if (!ImGui::IsItemDeactivatedAfterEdit())
    {
        return;
    }

    const auto& command_manager = editor_ctx.command_manager;
    const auto& gizmo = editor_ctx.gizmo;

    auto command = std::make_unique<ScaleObjectsCommand>(
        editor_ctx, RouteObjects{object}, gizmo->get_curr_pos(),
        total_scale);
    command_manager->push(std::move(command));

    dragging = false;
}

void EditorGui::add_ttf_font(const char* filename, float size_pixels,
                             const ImFontConfig* font_cfg,
                             const ImWchar* glyph_ranges)
{
    ImGuiIO& io = ImGui::GetIO();
    const FileSystem& fs = FileSystem::getInstance();
    const std::string font_path = fs.combinePath(fs.getFontsDir(), filename);
    io.Fonts->AddFontFromFileTTF(font_path.c_str(), size_pixels, font_cfg,
                                 glyph_ranges);
}

void EditorGui::draw_main_menu_bar() const
{
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
                ImGuiFileDialog::Instance()->OpenDialog(
                    "LoadRouteKey", "Load route", nullptr, config);
            }

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void EditorGui::draw_status_bar() const
{
    ImGui::SetNextWindowPos(
        ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y
                                    - ImGui::GetFrameHeight() * 1.5));

    ImGui::SetNextWindowSize(
        ImVec2(viewport->Size.x, ImGui::GetFrameHeight() * 1.5));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar
                             | ImGuiWindowFlags_NoScrollbar
                             | ImGuiWindowFlags_NoResize;

    const auto& state_manager = editor_ctx.state_manager;

    if (ImGui::Begin("StatusBar", nullptr, flags))
    {
        state_manager->get_curr_state()->fill_status_bar();
        ImGui::End();
    }
}

void EditorGui::draw_invalid_route_popup() const
{
    if (ImGui::BeginPopupModal("InvalidRoute", nullptr,
                               ImGuiWindowFlags_NoTitleBar
                                   | ImGuiWindowFlags_NoResize))
    {
        ImGui::Text("Invalid route!\n"
                    "Route must contain:\n"
                    "models/\n"
                    "textures/\n"
                    "topology/\n"
                    "objects.ref");

        if (ImGui::Button("OK", ImVec2(-FLT_MIN, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
