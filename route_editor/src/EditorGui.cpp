#include "EditorGui.h"

#include "Action.h"
#include "EditorState.h"
#include "KeyBindings.h"
// #include "ObjectProperties.h"
#include "ObjectSelector.h"
#include "Route.h"
#include "SceneGraph.h"
#include "Settings.h"
#include "filesystem.h"
// #include "rail-signal.h"
// #include "switch.h"
#include "topology.h"
// #include "track.h"
// #include "trajectory.h"
// #include "vec3.h"

#include <vsg/app/ProjectionMatrix.h>
#include <vsg/core/ref_ptr.h>
// #include <vsg/maths/common.h>
// #include <vsg/maths/mat4.h>
// #include <vsg/maths/transform.h>
// #include <vsg/nodes/MatrixTransform.h>
#include <vsg/maths/vec3.h>
#include <vsgImGui/imgui.h>

#include <cassert>
#include <cctype>
// #include <cstddef>
#include <filesystem>
#include <set>
#include <string>

EditorGui::EditorGui(
    settings_t& settings,
    EditorState& editor_state,
    const KeyBindings& key_bindings,
    vsg::ref_ptr<vsg::Perspective> perspective,
    vsg::ref_ptr<SceneGraph> scene_graph,
    vsg::ref_ptr<ObjectSelector> object_selector,
    std::string& route_directory
)
    : settings(settings)
    , editor_state(editor_state)
    , key_bindings(key_bindings)
    , perspective(perspective)
    , scene_graph(scene_graph)
    , object_selector(object_selector)
    , route_directory(route_directory)
{
    assert(perspective);
    assert(scene_graph);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    const FileSystem& fs = FileSystem::getInstance();

    const char* const font_name = "JetBrainsMono-Regular.ttf";
    const auto font_path = fs.combinePath(fs.getFontsDir(), font_name);

    io.Fonts->AddFontFromFileTTF(font_path.c_str(), settings.gui_font_size,
        nullptr, io.Fonts->GetGlyphRangesCyrillic());

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    if (!settings.is_gui_editable)
    {
        window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    }
}

void EditorGui::record(vsg::CommandBuffer& command_buffer) const
{
    (void)command_buffer;

    switch (editor_state)
    {
        case EditorState::SELECT_ROUTE:
        {
            select_route();

            return;
        }
        default:
        {
            ImGui::Begin("Settings", nullptr, window_flags);
            ImGui::Checkbox("Show objects.ref", &settings.show_objects_ref);
            ImGui::Checkbox("Show route1.map", &settings.show_route_map);
            ImGui::Checkbox("Show controls", &settings.show_controls);
            ImGui::Checkbox("Show camera settings", &settings.show_camera_settings);
            ImGui::Checkbox("Show topology", &settings.show_topology);
            ImGui::End();

            // ImGui::ShowDemoWindow();

            if (settings.show_objects_ref)
            {
                show_objects_ref();
            }

            if (settings.show_route_map)
            {
                show_route_map();
            }

            if (settings.show_controls)
            {
                show_key_bindings();
            }

            if (settings.show_camera_settings)
            {
                show_camera_settings();
            }

            if (settings.show_topology)
            {
                show_topology();
            }

            show_selected_object_properties();

            return;
        }
    }
}

void EditorGui::select_route() const
{
    using Path = std::filesystem::path;
    using DirIterator = std::filesystem::directory_iterator;

    // TODO: Rewrite in English
    // Сеты для будущего отображения папок и файлов в алфавитном порядке
    static std::set<Path> files;
    static std::set<Path> directories;

    const auto change_route_directory = [&](const Path& path) -> void
    {
        route_directory = path.string();

        files.clear();
        directories.clear();

        for (const auto& dir_entry : DirIterator(route_directory))
        {
            if (dir_entry.is_directory())
            {
                directories.emplace(dir_entry);
            }
            else
            {
                files.emplace(dir_entry);
            }
        }
    };

    // TODO: Rewrite in English
    // Вызывается при первом запуске select_route
    if (route_directory.empty())
    {
        change_route_directory(FileSystem::getInstance().getRouteRootDir());
    }

    ImGui::Begin("Select Route", nullptr, window_flags);

    ImGui::Text("Select route:");

    // TODO: Rewrite in English
    // Выводит путь к текущей выбранной папке и позволяет подтвердить ее
    ImGui::Text("Current: %s", route_directory.c_str());
    ImGui::SameLine();
    if (ImGui::Button("OK"))
    {
        editor_state = EditorState::LOAD_ROUTE;
    }

    // TODO: Rewrite in English
    // Отдельно выводим кнопку для перехода на одну папку выше
    if (Path(route_directory).has_parent_path())
    {
        if (ImGui::Button(".."))
        {
            change_route_directory(Path(route_directory).parent_path());
        }
    }

    // TODO: Rewrite in English
    // Отображаем все папки как кнопки для перемещения по ним
    for (const Path& directory : directories)
    {
        if (ImGui::Button(directory.filename().string().c_str()))
        {
            change_route_directory(directory);

            break;
        }
    }

    // TODO: Rewrite in English
    // А все файлы просто как текст
    for (const Path& file : files)
    {
        ImGui::Text("%s", file.filename().c_str());
    }

    ImGui::End();
}

void EditorGui::show_objects_ref() const
{
    ImGui::Begin("objects_ref", nullptr, window_flags);

    if (!scene_graph->get_route())
    {
        ImGui::Text("There is no route yet");
        ImGui::End();
        return;
    }

    if (ImGui::BeginTable("objects_ref_table", 2,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg))
    {
        const auto& objects_ref = scene_graph->get_route()->get_objects_ref();
        for (const auto& [label, relative_path] : objects_ref)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", label.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", relative_path.c_str());
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void EditorGui::show_route_map() const
{
    assert(scene_graph->get_route());

    ImGui::Begin("route1.map", nullptr, window_flags);

    if (!scene_graph->get_route())
    {
        ImGui::Text("There is no route yet");
        ImGui::End();
        return;
    }

    if (ImGui::BeginTable("route_map_table", 7,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg))
    {
        const auto& route_map = scene_graph->get_route()->get_route_map();
        for (const auto& [label, transform] : route_map)
        {
            const vsg::vec3 translation = transform.first;
            const vsg::vec3 rotation_deg = transform.second;

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
            label += std::toupper(key_bindings[i]);
            ImGui::Text("%s", label.c_str());
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void EditorGui::show_camera_settings() const
{
    ImGui::Begin("Camera Settings", nullptr, window_flags);

    // TODO: Replace double on float

    ImGui::Text("Move speed:");
    float move_speed = static_cast<float>(settings.camera_move_speed);
    if (ImGui::SliderFloat("##move_speed", &move_speed,
        settings.min_camera_move_speed, settings.max_camera_move_speed))
    {
        settings.camera_move_speed = move_speed;
    }

    ImGui::Text("Rotate speed:");
    float rotate_speed = static_cast<float>(settings.camera_rotate_speed);
    if (ImGui::SliderFloat("##rotate_speed", &rotate_speed,
        settings.min_camera_rotate_speed, settings.max_camera_rotate_speed))
    {
        settings.camera_rotate_speed = rotate_speed;
    }

    ImGui::Text("Zoom power:");
    float zoom_power = static_cast<float>(settings.camera_zoom_power);
    if (ImGui::SliderFloat("##zoom_power", &zoom_power,
        settings.min_camera_zoom_power, settings.max_camera_zoom_power))
    {
        settings.camera_zoom_power = zoom_power;
    }

    ImGui::Text("FovY:");
    float fovy = static_cast<float>(perspective->fieldOfViewY);
    if (ImGui::SliderFloat("##fovy", &fovy, settings.fovy_min,
        settings.fovy_max))
    {
        perspective->fieldOfViewY = fovy;
    }

    ImGui::End();
}

void EditorGui::show_topology() const
{
    ImGui::Begin("Topology", nullptr, window_flags);

    const auto route = scene_graph->get_route();
    if (!route)
    {
        ImGui::Text("There is no route yet");
        ImGui::End();
        return;
    }

    const auto& topology = route->get_topology();
    if (!topology)
    {
        ImGui::Text("There is no topology yet");
        ImGui::End();
        return;
    }

    const auto route_name = topology->getRouteName().toStdString();
    ImGui::Text("Route name: %s", route_name.c_str());

    // if (ImGui::CollapsingHeader("Trajectories"))
    // {
    //     const auto* trajectories = (*topology).getTrajectoriesList();
    //     for (auto it = trajectories->begin(); it != trajectories->end(); ++it)
    //     {
    //         Trajectory* const trajectory = *it;
    //         if (ImGui::TreeNode(trajectory->getName().toStdString().c_str()))
    //         {
    //             ImGui::Text("%17s%12s%12s%12s%12s", "begin.x", "begin.y", "begin.z", "rail_coord", "traj_coord");
    //             ImGui::Separator();
    //             const auto& tracks = trajectory->getTracks();
    //             for (std::size_t i = 0; i < tracks.size(); ++i)
    //             {
    //                 const track_t& track = tracks[i];
    //                 ImGui::Text("[%2zu]:%12.3f%12.3f%12.3f%12.3f%12.3f", i, track.begin_point.x, track.begin_point.y,
    //                     track.begin_point.z, track.railway_coord0, track.traj_coord);
    //             }

    //             ImGui::TreePop();
    //         }
    //     }
    // }

    // if (ImGui::CollapsingHeader("Switches"))
    // {
    //     const auto& connectors = *topology.getConnectorsList();
    //     for (auto it = connectors.begin(); it != connectors.end(); ++it)
    //     {
    //         const Switch* const switch_ = dynamic_cast<const Switch*>(it.value());
    //         if (!switch_)
    //         {
    //             continue;
    //         }

    //         if (ImGui::TreeNode(switch_->getName().toStdString().c_str()))
    //         {
    //             // if (const Trajectory* const bwd_plus_traj = switch_->getBwdPlusTraj())
    //             // {
    //             //     ImGui::Text("bwdPlusTraj: %s", bwd_plus_traj->getName().toStdString().c_str());
    //             // }

    //             if (const Trajectory* const fwd_plus_traj = switch_->getFwdTraj())
    //             {
    //                 ImGui::Text("fwdPlusTraj: %s", fwd_plus_traj->getName().toStdString().c_str());
    //             }

    //             // if (const Signal* const bwd_signal = switch_->getSignalBwd())
    //             // {
    //             //     ImGui::Text("SignalLiterBwd: %s", bwd_signal->getLetter().toStdString().c_str());
    //             //     ImGui::Text("SignalModelBwd: %s", bwd_signal->getSignalModel().toStdString().c_str());
    //             //     const dvec3 rel_pos = bwd_signal->getRelPosition();
    //             //     const dvec3 rel_rot = bwd_signal->getRelRotation();
    //             //     ImGui::Text("RelPosVectorBwd:%8.3f%8.3f%8.3f", rel_pos.x, rel_pos.y, rel_pos.z);
    //             //     ImGui::Text("RelRotVectorBwd:%8.3f%8.3f%8.3f", rel_rot.x, rel_rot.y, rel_rot.z);
    //             // }

    //             // if (const Signal* const fwd_signal = switch_->getSignalFwd())
    //             // {
    //             //     ImGui::Text("SignalLiterFwd: %s", fwd_signal->getLetter().toStdString().c_str());
    //             //     ImGui::Text("SignalModelFwd: %s", fwd_signal->getSignalModel().toStdString().c_str());
    //             //     const dvec3 rel_pos = fwd_signal->getRelPosition();
    //             //     const dvec3 rel_rot = fwd_signal->getRelRotation();
    //             //     ImGui::Text("RelPosVectorFwd:%8.3f%8.3f%8.3f", rel_pos.x, rel_pos.y, rel_pos.z);
    //             //     ImGui::Text("RelRotVectorFwd:%8.3f%8.3f%8.3f", rel_rot.x, rel_rot.y, rel_rot.z);
    //             // }

    //             // if (const Trajectory* const bwd_minus_traj = switch_->getBwdMinusTraj())
    //             // {
    //             //     ImGui::Text("bwdMinusTraj: %s", bwd_minus_traj->getName().toStdString().c_str());
    //             //     ImGui::Text("state_bwd: %d", switch_->getStateBwd());
    //             // }

    //             // if (const Trajectory* const fwd_minus_traj = switch_->getFwdMinusTraj())
    //             // {
    //             //     ImGui::Text("fwdMinusTraj: %s", fwd_minus_traj->getName().toStdString().c_str());
    //             //     ImGui::Text("state_fwd: %d", switch_->getStateFwd());
    //             // }

    //             ImGui::TreePop();
    //         }
    //     }
    // }

    ImGui::End();
}

void EditorGui::show_selected_object_properties() const
{
    // if (!selected_object)
    // {
    //     return;
    // }

    // if (auto matrix_transform = *selected_object)
    // {
    //     ObjectProperties properties;
    //     if (matrix_transform->getValue("properties", properties))
    //     {
    //         ImGui::Begin("Properties", nullptr, window_flags);

    //         ImGui::Text("name: %s", properties.name.c_str());

    //         ImGui::Text("translation:");
    //         if (ImGui::SliderFloat("x##translation", &properties.translation.x, -100.0f, 100.0f))
    //         {
    //             update_selected_object_matrix(properties, matrix_transform);
    //         }
    //         if (ImGui::SliderFloat("y##translation", &properties.translation.y, -100.0f, 10000.0f))
    //         {
    //             update_selected_object_matrix(properties, matrix_transform);
    //         }
    //         if (ImGui::SliderFloat("z##translation", &properties.translation.z, -100.0f, 100.0f))
    //         {
    //             update_selected_object_matrix(properties, matrix_transform);
    //         }

    //         ImGui::Text("rotation:");
    //         if (ImGui::SliderFloat("x##rotation", &properties.rotation_deg.x, -180.0f, 180.0f))
    //         {
    //             update_selected_object_matrix(properties, matrix_transform);
    //         }
    //         if (ImGui::SliderFloat("y##rotation", &properties.rotation_deg.y, -180.0f, 180.0f))
    //         {
    //             update_selected_object_matrix(properties, matrix_transform);
    //         }
    //         if (ImGui::SliderFloat("z##rotation", &properties.rotation_deg.z, -180.0f, 180.0f))
    //         {
    //             update_selected_object_matrix(properties, matrix_transform);
    //         }

    //         ImGui::End();
    //     }
    // }
}
