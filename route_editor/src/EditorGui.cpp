#include "EditorGui.h"

#include "Action.h"
#include "commands/AddObject.h"
#include "CameraHandler.h"
#include "commands/Command.h"
#include "commands/CommandList.h"
#include "EditorContext.h"
#include "EditorState.h"
#include "Gizmo.h"
#include "KeyBinding.h"
#include "commands/TranslateObjects.h"
#include "ObjectSelector.h"
#include "Route.h"
#include "RouteObject.h"
#include "SceneGraph.h"
#include "Settings.h"
#include "filesystem.h"
#include "rail-signal.h"
#include "switch.h"
#include "topology-defines.h"
#include "topology.h"
#include "track.h"
#include "ImGuiFileDialog.h"
#include "trajectory.h"
#include "vec3.h"

#include <vsg/app/ProjectionMatrix.h>
#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/common.h>
#include <vsg/maths/quat.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsgImGui/imgui.h>

#include <cassert>
#include <cctype>
#include <cfloat>
#include <climits>
#include <string>

static constexpr float MAX_DRAG = FLT_MAX / static_cast<float>(INT_MAX);

EditorGui::EditorGui(EditorContext& context)
    : context(context)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    const FileSystem& fs = FileSystem::getInstance();

    const char* const font_name = "JetBrainsMono-Regular.ttf";
    const auto font_path = fs.combinePath(fs.getFontsDir(), font_name);

    io.Fonts->AddFontFromFileTTF(font_path.c_str(), context.settings.gui_font_size,
        nullptr, io.Fonts->GetGlyphRangesCyrillic());

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    if (!context.settings.is_gui_editable)
    {
        window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    }

    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 6.0f;
    style.FrameBorderSize = 1.0f;
    style.WindowRounding = 6.0f;
    style.ScrollbarSize = 16.0f;
    style.GrabMinSize = 16.0f;
    style.GrabRounding = 6.0f;
}

void EditorGui::record(vsg::CommandBuffer& command_buffer) const
{
    (void)command_buffer;

    switch (context.state)
    {
        case EditorState::SELECT_ROUTE:
        {
            select_route();

            return;
        }
        default:
        {
            ImGui::Begin("Settings", nullptr, window_flags);
            ImGui::Checkbox("Show objects.ref", &context.settings.show_objects_ref);
            ImGui::Checkbox("Show route1.map", &context.settings.show_route_map);
            ImGui::Checkbox("Show controls", &context.settings.show_controls);
            ImGui::Checkbox("Show camera settings", &context.settings.show_camera_settings);
            ImGui::Checkbox("Show topology", &context.settings.show_topology);
            ImGui::End();

            // ImGui::ShowDemoWindow();

            if (context.settings.show_objects_ref)
            {
                show_objects_ref();
            }

            if (context.settings.show_route_map)
            {
                show_route_map();
            }

            show_stations_conf();
            show_waypoints_conf();

            if (context.settings.show_controls)
            {
                show_key_bindings();
            }

            if (context.settings.show_camera_settings)
            {
                show_camera_settings();
            }

            if (context.settings.show_topology)
            {
                show_topology();
            }

            show_selected_objects_properties();

            ImGui::Begin("Commands");
            auto active = context.commands.get_active();
            auto curr = context.commands.get_tail();
            while (curr)
            {
                if (curr == active)
                {
                    ImGui::TextColored(ImVec4{0.2f, 1.0f, 0.3f, 1.0f}, "%s", curr->command->get_description());
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

            return;
        }
    }
}

void EditorGui::select_route() const
{
    static bool dialog_opened = false;
    if (!dialog_opened)
    {
        IGFD::FileDialogConfig config;
        config.path = "../routes";
        ImGuiFileDialog::Instance()->OpenDialog(
            "select_route", "Select route", nullptr, config);
        dialog_opened = true;
    }

    if (ImGuiFileDialog::Instance()->Display("select_route"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            context.route_dir = ImGuiFileDialog::Instance()->GetCurrentPath();
            context.state = EditorState::LOAD_ROUTE;
        }

        ImGuiFileDialog::Instance()->Close();
    }
}

void EditorGui::show_objects_ref() const
{
    ImGui::Begin("objects_ref", nullptr, window_flags);

    if (!context.route)
    {
        ImGui::Text("There is no route yet");
        ImGui::End();
        return;
    }

    if (ImGui::BeginTable("objects_ref_table", 2,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg))
    {
        for (const auto& [label, ref] : context.objects_ref)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Button(label.c_str()))
            {
                const auto object = RouteObject::create(context, ref.paged_lod,
                    label, static_cast<vsg::vec3>(context.look_at->eye) +
                    static_cast<vsg::vec3>(context.camera_handler->get_front())
                        * 20.0f,
                    vsg::vec3(0.0f, 0.0f, 0.0f)
                );

                context.commands.push(new AddObject(
                    context, object), true);
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
    assert(context.route);

    ImGui::Begin("route1.map", nullptr, window_flags);

    if (!context.route)
    {
        ImGui::Text("There is no route yet");
        ImGui::End();
        return;
    }

    if (ImGui::BeginTable("route_map_table", 7,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg))
    {
        for (const auto& [label, transforms] : context.route_map)
        {
            for (const auto& transform : transforms)
            {
                const vsg::vec3 translation = transform.translation;
                const vsg::vec3 rotation_deg = transform.rotation_deg;

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", label.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%10.3f", translation.x);
                ImGui::TableNextColumn();
                ImGui::Text("%10.3f", translation.y);
                ImGui::TableNextColumn();
                ImGui::Text("%10.3f", translation.z);
                ImGui::TableNextColumn();
                ImGui::Text("%10.3f", rotation_deg.x);
                ImGui::TableNextColumn();
                ImGui::Text("%10.3f", rotation_deg.y);
                ImGui::TableNextColumn();
                ImGui::Text("%10.3f", rotation_deg.z);
            }
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void EditorGui::show_stations_conf() const
{
    ImGui::Begin("stations.conf", nullptr, window_flags);

    if (ImGui::BeginTable("stations_conf_table", 4,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg))
    {
        for (const auto& [label, translation] : context.stations_conf)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Button(label.c_str()))
            {
                context.look_at->eye = static_cast<vsg::dvec3>(translation)
                    + vsg::dvec3(0.0, 0.0, 50.0);

                context.look_at->center = context.look_at->eye
                    + context.camera_handler->get_front();
            }
            ImGui::TableNextColumn();
            ImGui::Text("%10.3f", translation.x);
            ImGui::TableNextColumn();
            ImGui::Text("%10.3f", translation.y);
            ImGui::TableNextColumn();
            ImGui::Text("%10.3f", translation.z);
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void EditorGui::show_waypoints_conf() const
{
    ImGui::Begin("waypoints.conf", nullptr, window_flags);

    if (ImGui::BeginTable("waypoints_conf_table", 5,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg))
    {
        for (const auto& [label, data] : context.waypoints_conf)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Button(label.c_str()))
            {
                const traj_list_t* const traj_list =
                    context.topology->getTrajectoriesList();

                const QString traj_name = QString::fromStdString(
                    data.trajectory_name);

                auto found_it = traj_list->find(traj_name);
                if (found_it == traj_list->end())
                {
                    // TODO: Replace on Journal
                    std::fprintf(stderr, "Failed to find trajectory %s\n",
                        data.trajectory_name.c_str());
                }
                else
                {
                    Trajectory* const trajectory = *found_it;

                    auto pd = trajectory->getPosition(data.coord, data.direction);

                    const dvec3 pos = pd.position;

                    double h = 5.0;

                    context.look_at->eye = vsg::dvec3(pos.x + pd.up.x * h,
                                                      pos.y + pd.up.y * h,
                                                      pos.z + pd.up.z * h);

                    context.look_at->center = context.look_at->eye
                        + static_cast<vsg::dvec3>(context.camera_handler->get_front());
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
    ImGui::Begin("Key Bindings", nullptr, window_flags);

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

            static const std::map<EditorKeyModifier, const char*> test_map = {
                {EDITOR_KEY_MODIFIER_SHIFT_L, "LShift"},
                {EDITOR_KEY_MODIFIER_SHIFT_R, "RShift"},
                {EDITOR_KEY_MODIFIER_SHIFT_ANY, "Shift"},
                {EDITOR_KEY_MODIFIER_CTRL_L, "LCtrl"},
                {EDITOR_KEY_MODIFIER_CTRL_R, "RCtrl"},
                {EDITOR_KEY_MODIFIER_CTRL_ANY, "Ctrl"},
                {EDITOR_KEY_MODIFIER_ALT_L, "LAlt"},
                {EDITOR_KEY_MODIFIER_ALT_R, "RAlt"},
                {EDITOR_KEY_MODIFIER_ALT_ANY, "Alt"}
            };

            for (const auto& [modifier, name] : test_map)
            {
                if (context.settings.key_bindings[i].modifiers & modifier)
                {
                    label += name;
                    label += " + ";
                }
            }

            label += std::toupper(context.settings.key_bindings[i].key);
            ImGui::Text("%s", label.c_str());
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void EditorGui::show_camera_settings() const
{
    ImGui::Begin("Camera Settings", nullptr, window_flags);

    ImGui::Text("Move speed:");
    ImGui::DragFloat("##move_speed", &context.settings.camera_move_speed,
        1.0f, 1.0f, MAX_DRAG);

    ImGui::Text("Rotate speed:");
    ImGui::DragFloat("##rotate_speed", &context.settings.camera_rotate_speed,
        1.0f, 1.0f, MAX_DRAG);

    ImGui::Text("Zoom power:");
    ImGui::DragFloat("##zoom_power", &context.settings.camera_zoom_power,
        1.0f, 1.0f, MAX_DRAG);

    ImGui::Text("FovY:");
    float fovy = static_cast<float>(context.perspective->fieldOfViewY);
    if (ImGui::SliderFloat("##fovy", &fovy, context.settings.fovy_min,
        context.settings.fovy_max))
    {
        context.perspective->fieldOfViewY = fovy;
    }

    ImGui::End();
}

void EditorGui::show_topology() const
{
    ImGui::Begin("Topology", nullptr, window_flags);

    const auto route = context.route;
    if (!route)
    {
        ImGui::Text("There is no route yet");
        ImGui::End();
        return;
    }

    if (!context.topology)
    {
        ImGui::Text("There is no topology yet");
        ImGui::End();
        return;
    }

    const auto route_name = context.topology->getRouteName().toStdString();
    ImGui::Text("Route name: %s", route_name.c_str());

    if (ImGui::CollapsingHeader("Trajectories"))
    {
        const auto* trajectories = context.topology->getTrajectoriesList();
        for (const Trajectory* trajectory : *trajectories)
        {
            if (ImGui::TreeNode(trajectory->getName().toStdString().c_str()))
            {
                ImGui::Text("%17s%12s%12s%12s%12s", "begin.x", "begin.y",
                    "begin.z", "rail_coord", "traj_coord");

                ImGui::Separator();

                const auto& tracks = trajectory->getTracks();
                const auto tracks_size = tracks.size();

                for (auto i = decltype(tracks_size){0}; i < tracks_size; ++i)
                {
                    const track_t& track = tracks[i];
                    ImGui::Text("[%2zu]:%12.3f%12.3f%12.3f%12.3f%12.3f", i,
                        track.begin_point.x, track.begin_point.y,
                        track.begin_point.z, track.railway_coord0,
                        track.traj_coord);
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

                const dvec3 rel_pos = signal->getRelPos();
                const dvec3 rel_rot = signal->getRelRot();

                ImGui::Text("RelPos%s: %8.3f %8.3f %8.3f", type,
                    rel_pos.x, rel_pos.y, rel_pos.z);

                ImGui::Text("RelRot%s: %8.3f %8.3f %8.3f", type,
                    rel_rot.x, rel_rot.y, rel_rot.z);
            }
        };

        const auto* connectors = context.topology->getConnectorsList();
        for (auto it = connectors->begin(); it != connectors->end(); ++it)
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
    if (!context.object_selector)
    {
        return;
    }

    const auto& selected_objects = context.selected_objects;
    if (selected_objects.empty())
    {
        return;
    }

    ImGui::Begin("Selected objects", nullptr, window_flags);

    int i = 0;

    static bool dragging = false;
    static vsg::vec3 total_translation;

    const auto save_matrixes = [&]() -> void
    {
        for (const auto& object : selected_objects)
        {
            object->save_matrix();
        }
    };

    for (const auto& object : selected_objects)
    {
        ImGui::Text("label: %s", object->label.c_str());

        std::string label = "translation##" + std::to_string(i);

        vsg::vec3 translation = object->get_translation();
        if (ImGui::DragFloat3(label.c_str(), translation.data()))
        {
            if (!dragging)
            {
                total_translation = {0.0f, 0.0f, 0.0f};
                save_matrixes();
                dragging = true;
            }
            total_translation += translation;

            object->set_translation(translation);
        }
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            context.commands.push(new TranslateObjects(context, total_translation), false);
            dragging = false;
        }

        label = "rotation##" + std::to_string(i);

        vsg::vec3 rotation_deg = object->get_rotation_deg();
        if (ImGui::DragFloat3(label.c_str(), rotation_deg.data(), 0.2f))
        {
            object->set_rotation_deg(rotation_deg);
        }

        label = "scale##" + std::to_string(i);

        vsg::vec3 scale = object->get_scale();
        if (ImGui::DragFloat3(label.c_str(), scale.data(), 0.01f))
        {
            object->set_scale(scale);
        }

        ++i;
    }

    ImGui::End();
}
