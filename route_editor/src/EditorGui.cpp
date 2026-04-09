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
#include "commands/RotateObjects.h"
#include "commands/ScaleObjects.h"
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

#include <algorithm>
// #include <filesystem>
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
#include <string>

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

EditorGui::EditorGui(EditorContext& context)
    : context_(context)
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
        window_flags_ |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
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

    switch (context_.state)
    {
        case EditorState::SELECT_ROUTE:
        {
            select_route();

            return;
        }
        default:
        {
            ImGui::Begin("Settings", nullptr, window_flags_);
            ImGui::Checkbox("Show objects.ref", &context_.settings.show_objects_ref);
            ImGui::Checkbox("Show route1.map", &context_.settings.show_route_map);
            ImGui::Checkbox("Show controls", &context_.settings.show_controls);
            ImGui::Checkbox("Show camera settings", &context_.settings.show_camera_settings);
            ImGui::Checkbox("Show topology", &context_.settings.show_topology);
            ImGui::End();

            // ImGui::ShowDemoWindow();

            if (context_.settings.show_objects_ref)
            {
                show_objects_ref();
            }

            if (context_.settings.show_route_map)
            {
                show_route_map();
            }

            show_stations_conf();
            show_waypoints_conf();

            if (context_.settings.show_controls)
            {
                show_key_bindings();
            }

            if (context_.settings.show_camera_settings)
            {
                show_camera_settings();
            }

            if (context_.settings.show_topology)
            {
                show_topology();
            }

            show_selected_objects_properties();

            // enum class ImportState
            // {
            //     INITIAL,
            //     SELECT_OBJECT,
            //     TEST,
            //     COPY_OBJECT,
            //     LABELING
            // };

            // static ImportState import_state = ImportState::INITIAL;
            // static std::string file_path;
            // static std::string dir;

            // if (import_state == ImportState::INITIAL)
            // {
            //     ImGui::Begin("Import object");
            //     if (ImGui::Button("Import object"))
            //     {
            //         IGFD::FileDialogConfig config;
            //         config.path = context_.route_dir;
            //         ImGuiFileDialog::Instance()->OpenDialog("select_object",
            //             "Select object", ".gltf", config);
            //         import_state = ImportState::SELECT_OBJECT;
            //     }
            //     ImGui::End();
            // }
            // else if (import_state == ImportState::SELECT_OBJECT)
            // {
            //     if (ImGuiFileDialog::Instance()->Display("select_object"))
            //     {
            //         if (ImGuiFileDialog::Instance()->IsOk())
            //         {
            //             file_path = ImGuiFileDialog::Instance()->GetFilePathName();
            //             import_state = ImportState::TEST;
            //         }

            //         ImGuiFileDialog::Instance()->Close();
            //     }
            // }
            // else if (import_state == ImportState::TEST)
            // {
            //     IGFD::FileDialogConfig config;
            //     config.path = context_.route_dir;
            //     ImGuiFileDialog::Instance()->OpenDialog("select_dir",
            //         "Select directory", nullptr, config);
            //     import_state = ImportState::COPY_OBJECT;
            // }
            // else if (import_state == ImportState::COPY_OBJECT)
            // {
            //     if (ImGuiFileDialog::Instance()->Display("select_dir"))
            //     {
            //         if (ImGuiFileDialog::Instance()->IsOk())
            //         {
            //             dir = ImGuiFileDialog::Instance()->GetCurrentPath();
            //             try
            //             {
            //                 std::filesystem::copy(file_path, dir);
            //             }
            //             catch (const std::filesystem::filesystem_error &e)
            //             {
            //                 fprintf(stderr, "%s\n", e.what());
            //             }
            //             import_state = ImportState::LABELING;
            //         }

            //         ImGuiFileDialog::Instance()->Close();
            //     }
            // }
            // else if (import_state == ImportState::LABELING)
            // {
            //     ImGui::Begin("Import object");
            //     static char buf[128];
            //     ImGui::InputText("label", buf, 128);
            //     ImGui::Text("%s", file_path.c_str());
            //         ImGui::Text("%s", dir.c_str());
            //     if (ImGui::Button("OK##import"))
            //     {
            //         import_state = ImportState::INITIAL;
            //     }
            //     ImGui::End();
            // }

            ImGui::Begin("Commands");
            auto active = context_.commands.get_active();
            auto curr = context_.commands.get_tail();
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
            context_.route_dir = ImGuiFileDialog::Instance()->GetCurrentPath();
            context_.state = EditorState::LOAD_ROUTE;
        }

        ImGuiFileDialog::Instance()->Close();
    }
}

void EditorGui::show_objects_ref() const
{
    ImGui::Begin("objects_ref", nullptr, window_flags_);

    if (!context_.route)
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
                const auto object = RouteObject::create(context_, ref.paged_lod,
                    label, context_.look_at->eye +
                        context_.camera_handler->get_front() * 20.0,
                    vsg::dvec3(0.0, 0.0, 0.0)
                );

                context_.commands.push(new AddObject(
                    context_, object), true);
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
    assert(context_.route);

    ImGui::Begin("route1.map", nullptr, window_flags_);

    if (!context_.route)
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
    ImGui::Begin("stations.conf", nullptr, window_flags_);

    if (ImGui::BeginTable("stations_conf_table", 4,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg))
    {
        for (const auto& [label, translation] : context_.stations_conf)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Button(label.c_str()))
            {
                context_.look_at->eye = translation +
                    vsg::dvec3(0.0, 0.0, 50.0);

                context_.look_at->center = context_.look_at->eye
                    + context_.camera_handler->get_front();
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

                    context_.look_at->eye = vsg::dvec3(pos.x + pd.up.x * h,
                                                      pos.y + pd.up.y * h,
                                                      pos.z + pd.up.z * h);

                    context_.look_at->center = context_.look_at->eye
                        + context_.camera_handler->get_front();
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

            static const std::map<EditorKeyModifier, const char*> test_map = {
                {EDITOR_KEY_MODIFIER_SHIFT, "Shift"},
                {EDITOR_KEY_MODIFIER_CTRL, "Ctrl"},
                {EDITOR_KEY_MODIFIER_ALT, "Alt"}
            };

            for (const auto& [modifier, name] : test_map)
            {
                if (context_.settings.key_bindings[i].modifiers & modifier)
                {
                    label += name;
                    label += " + ";
                }
            }

            label += std::toupper(context_.settings.key_bindings[i].key);
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
    drag_double("##move_speed", &context_.settings.camera_move_speed, &min);

    ImGui::Text("Rotate speed:");
    drag_double("##rotate_speed", &context_.settings.camera_rotate_speed, &min);

    ImGui::Text("Zoom power:");
    drag_double("##zoom_power", &context_.settings.camera_zoom_power, &min);

    ImGui::Text("FovY:");

    settings_t& settings = context_.settings;
    if (ImGui::SliderScalar("##fovy", ImGuiDataType_Double, &settings.fovy,
        &settings.fovy_min, &settings.fovy_max, "%.3f"))
    {
        context_.perspective->fieldOfViewY = settings.fovy;
    }

    ImGui::End();
}

void EditorGui::show_topology() const
{
    ImGui::Begin("Topology", nullptr, window_flags_);

    const auto route = context_.route;
    if (!route)
    {
        ImGui::Text("There is no route yet");
        ImGui::End();
        return;
    }

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
                ImGui::Text("%17s%12s%12s%12s%12s", "begin.x", "begin.y",
                    "begin.z", "rail_coord", "traj_coord");

                ImGui::Separator();

                const auto& tracks = trajectory->getTracks();
                const auto tracks_size = tracks.size();

                for (auto i = decltype(tracks_size){0}; i < tracks_size; ++i)
                {
                    const track_t& track = tracks[i];
                    ImGui::Text("[%4zu]:%12.3f%12.3f%12.3f%12.3f%12.3f", i,
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

    int i = 0;

    static bool dragging = false;
    static vsg::dvec3 total_translation;
    static vsg::dvec3 total_rotation_deg;
    static vsg::dvec3 total_scale;

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

        vsg::dvec3 translation = object->get_translation();
        if (drag_double3(label.c_str(), translation.data()))
        {
            if (!dragging)
            {
                total_translation = {0.0, 0.0, 0.0};
                save_matrixes();
                dragging = true;
            }
            total_translation += translation - object->get_translation();
            object->set_translation(translation);
        }

        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            context_.commands.push(new TranslateObjects(context_, {object},
                total_translation), false);
            dragging = false;
        }

        label = "rotation##" + std::to_string(i);

        constexpr double min_rot_deg = -360.0;
        constexpr double max_rot_deg = 360.0;
        vsg::dvec3 rotation_deg = object->get_rotation_deg();
        if (drag_double3(label.c_str(), rotation_deg.data(), 1.0f,
            &min_rot_deg, &max_rot_deg, ImGuiSliderFlags_WrapAround))
        {
            if (!dragging)
            {
                total_rotation_deg = {0.0, 0.0, 0.0};
                save_matrixes();
                dragging = true;
            }
            total_rotation_deg += rotation_deg - object->get_rotation_deg();
            object->set_rotation_deg(rotation_deg);
        }

        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            vsg::dvec3 axis;
            double radians;
            if (std::abs(total_rotation_deg.x) >= 1.0e-6)
            {
                axis = {1.0, 0.0, 0.0};
                radians = vsg::radians(total_rotation_deg.x);
            }
            else if (std::abs(total_rotation_deg.y) >= 1.0e-6)
            {
                axis = {0.0, 1.0, 0.0};
                radians = vsg::radians(total_rotation_deg.y);
            }
            else
            {
                axis = {0.0, 0.0, 1.0};
                radians = vsg::radians(total_rotation_deg.z);
            }
            context_.commands.push(new RotateObjects(context_, {object},
                context_.gizmo->get_curr_pos(), axis, radians), false);
            dragging = false;
        }

        label = "scale##" + std::to_string(i);

        const vsg::dvec3& prev_scale = object->get_scale();
        vsg::dvec3 scale = object->get_scale();
        if (drag_double3(label.c_str(), scale.data(), 0.01f))
        {
            if (vsg::length(scale) > 1.0e-6)
            {
                if (!dragging)
                {
                    total_scale = {1.0, 1.0, 1.0};
                    save_matrixes();
                    dragging = true;
                }
                total_scale *= {scale.x / prev_scale.x, scale.y / prev_scale.y,
                    scale.z / prev_scale.z};
                object->set_scale(scale);
            }
        }

        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            context_.commands.push(new ScaleObjects(context_, {object},
                context_.gizmo->get_curr_pos(), total_scale), false);
            dragging = false;
        }

        ++i;
    }

    ImGui::End();
}
