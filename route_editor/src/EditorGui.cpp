#include "EditorGui.h"

#include "Action.h"
#include "EditorParams.h"
#include "EditorState.h"
#include "ObjectProperties.h"
#include "Settings.h"
#include "filesystem.h"
#include "rail-signal.h"
#include "switch.h"
#include "topology.h"
#include "track.h"
#include "trajectory.h"
#include "vec3.h"

#include <vsg/app/ProjectionMatrix.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/common.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsgImGui/imgui.h>

#include <cctype>
#include <cstddef>
#include <filesystem>
#include <set>
#include <string>

static void update_selected_object_matrix(
    const ObjectProperties& properties,
    vsg::ref_ptr<vsg::MatrixTransform> matrix_transform
);

EditorGui::EditorGui(
    vsg::ref_ptr<EditorParams> editor_params,
    settings_t& settings,
    vsg::ref_ptr<vsg::Options> options
)
    : editor_params(editor_params)
    , settings(settings)
{
    (void)options;

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    FileSystem& fs = FileSystem::getInstance();
    const std::filesystem::path font_path = fs.combinePath(fs.getFontsDir(), "IosevkaNerdFont-Regular.ttf");

    io.Fonts->AddFontFromFileTTF(font_path.string().c_str(),
                                 settings.gui_font_size,
                                 nullptr,
                                 io.Fonts->GetGlyphRangesCyrillic());

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    if (!settings.is_gui_editable)
    {
        window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    }
}

void EditorGui::record(vsg::CommandBuffer& command_buffer) const
{
    (void)command_buffer;

    switch (*editor_params->editor_state)
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

            if (editor_params->show_demo_window)
            {
                ImGui::ShowDemoWindow();
            }

            if (settings.show_objects_ref)
            {
                show_objects_ref();
            }

            if (settings.show_route_map)
            {
                show_route_map();
            }

            show_selected_object_properties();

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

            return;
        }
    }
}

void EditorGui::select_route() const
{
    // Сеты для будущего отображения папок и файлов в алфавитном порядке
    static std::set<std::filesystem::path> files;
    static std::set<std::filesystem::path> directories;

    auto change_route_dir = [&](const std::filesystem::path& path) -> void {
        editor_params->route_dir = path;

        files.clear();
        directories.clear();

        for (const auto& dir_entry : std::filesystem::directory_iterator(editor_params->route_dir))
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

    // Вызывается при первом запуске select_route
    if (editor_params->route_dir.empty())
    {
        editor_params->route_dir = FileSystem::getInstance().getRouteRootDir();
        change_route_dir(editor_params->route_dir);
    }

    ImGui::Begin("Select Route", nullptr, window_flags);

    ImGui::Text("Select route:");

    // Выводит путь к текущей выбранной папке и позволяет подтвердить ее
    ImGui::Text("Current: %s", editor_params->route_dir.c_str());
    ImGui::SameLine();
    if (ImGui::Button("OK"))
    {
        *editor_params->editor_state = EditorState::LOAD_ROUTE;
    }

    // Отдельно выводим кнопку для перехода на одну папку выше
    if (editor_params->route_dir.has_parent_path())
    {
        if (ImGui::Button(".."))
        {
            change_route_dir(editor_params->route_dir.parent_path());
        }
    }

    // Отображаем все папки как кнопки для перемещния по ним
    for (const std::filesystem::path& directory : directories)
    {
        if (ImGui::Button(directory.filename().string().c_str()))
        {
            change_route_dir(directory);
            break;
        }
    }

    // А все файлы просто как текст
    for (const std::filesystem::path& file : files)
    {
        ImGui::Text("%s", file.filename().c_str());
    }

    ImGui::End();
}

void EditorGui::show_objects_ref() const
{
    ImGui::Begin("objects_ref", nullptr, window_flags);

    if (ImGui::BeginTable("objects_ref_table", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        for (const auto& [label, relative_path] : *editor_params->objects_ref)
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
    ImGui::Begin("route1.map", nullptr, window_flags);

    if (ImGui::BeginTable("route_map_table", 7, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        for (const auto& [label, transform] : *editor_params->route_map)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", label.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%.3f", transform.first.x);
            ImGui::TableNextColumn();
            ImGui::Text("%.3f", transform.first.y);
            ImGui::TableNextColumn();
            ImGui::Text("%.3f", transform.first.z);
            ImGui::TableNextColumn();
            ImGui::Text("%.3f", transform.second.x);
            ImGui::TableNextColumn();
            ImGui::Text("%.3f", transform.second.y);
            ImGui::TableNextColumn();
            ImGui::Text("%.3f", transform.second.z);
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void EditorGui::show_key_bindings() const
{
    ImGui::Begin("Key Bindings", nullptr, window_flags);

    if (ImGui::BeginTable("key_bindings_table", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        for (int i = 0; i < ACTION_TOTAL_COUNT; ++i)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", to_c_string(static_cast<Action>(i)));
            ImGui::TableNextColumn();
            std::string label;
            label += std::toupper(editor_params->key_bindings[i]);
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
    float move_speed = static_cast<float>(settings.camera_move_speed);
    if (ImGui::SliderFloat("##move_speed", &move_speed, settings.min_camera_move_speed, settings.max_camera_move_speed))
    {
        settings.camera_move_speed = move_speed;
    }

    ImGui::Text("Rotate speed:");
    float rotate_speed = static_cast<float>(settings.camera_rotate_speed);
    if (ImGui::SliderFloat("##rotate_speed", &rotate_speed, settings.min_camera_rotate_speed, settings.max_camera_rotate_speed))
    {
        settings.camera_rotate_speed = rotate_speed;
    }

    ImGui::Text("Zoom power:");
    float zoom_power = static_cast<float>(settings.camera_zoom_power);
    if (ImGui::SliderFloat("##zoom_power", &zoom_power, settings.min_camera_zoom_power, settings.max_camera_zoom_power))
    {
        settings.camera_zoom_power = zoom_power;
    }

    ImGui::Text("FovY:");
    float fovy = static_cast<float>(editor_params->perspective->fieldOfViewY);
    if (ImGui::SliderFloat("##fovy", &fovy, settings.fovy_min, settings.fovy_max))
    {
        editor_params->perspective->fieldOfViewY = fovy;
    }

    ImGui::End();
}

void EditorGui::show_topology() const
{
    Topology& topology = *editor_params->topology;
    if (topology.getRouteName().isEmpty())
    {
        return;
    }

    ImGui::Begin("Topology", nullptr, window_flags);

    ImGui::Text("Route name: %s", topology.getRouteName().toStdString().c_str());

    if (ImGui::CollapsingHeader("Trajectories"))
    {
        const auto& trajectories = *topology.getTrajectoriesList();
        for (auto it = trajectories.begin(); it != trajectories.end(); ++it)
        {
            Trajectory* const trajectory = it.value();
            if (ImGui::TreeNode(trajectory->getName().toStdString().c_str()))
            {
                ImGui::Text("%17s%12s%12s%12s%12s", "begin.x", "begin.y", "begin.z", "rail_coord", "traj_coord");
                ImGui::Separator();
                const auto& tracks = trajectory->getTracks();
                for (std::size_t i = 0; i < tracks.size(); ++i)
                {
                    const track_t& track = tracks[i];
                    ImGui::Text("[%2zu]:%12.3f%12.3f%12.3f%12.3f%12.3f", i, track.begin_point.x, track.begin_point.y,
                        track.begin_point.z, track.railway_coord0, track.traj_coord);
                }

                ImGui::TreePop();
            }
        }
    }

    if (ImGui::CollapsingHeader("Switches"))
    {
        const auto& connectors = *topology.getConnectorsList();
        for (auto it = connectors.begin(); it != connectors.end(); ++it)
        {
            const Switch* const switch_ = dynamic_cast<const Switch*>(it.value());
            if (!switch_)
            {
                continue;
            }

            if (ImGui::TreeNode(switch_->getName().toStdString().c_str()))
            {
                // if (const Trajectory* const bwd_plus_traj = switch_->getBwdPlusTraj())
                // {
                //     ImGui::Text("bwdPlusTraj: %s", bwd_plus_traj->getName().toStdString().c_str());
                // }

                if (const Trajectory* const fwd_plus_traj = switch_->getFwdTraj())
                {
                    ImGui::Text("fwdPlusTraj: %s", fwd_plus_traj->getName().toStdString().c_str());
                }

                // if (const Signal* const bwd_signal = switch_->getSignalBwd())
                // {
                //     ImGui::Text("SignalLiterBwd: %s", bwd_signal->getLetter().toStdString().c_str());
                //     ImGui::Text("SignalModelBwd: %s", bwd_signal->getSignalModel().toStdString().c_str());
                //     const dvec3 rel_pos = bwd_signal->getRelPosition();
                //     const dvec3 rel_rot = bwd_signal->getRelRotation();
                //     ImGui::Text("RelPosVectorBwd:%8.3f%8.3f%8.3f", rel_pos.x, rel_pos.y, rel_pos.z);
                //     ImGui::Text("RelRotVectorBwd:%8.3f%8.3f%8.3f", rel_rot.x, rel_rot.y, rel_rot.z);
                // }

                // if (const Signal* const fwd_signal = switch_->getSignalFwd())
                // {
                //     ImGui::Text("SignalLiterFwd: %s", fwd_signal->getLetter().toStdString().c_str());
                //     ImGui::Text("SignalModelFwd: %s", fwd_signal->getSignalModel().toStdString().c_str());
                //     const dvec3 rel_pos = fwd_signal->getRelPosition();
                //     const dvec3 rel_rot = fwd_signal->getRelRotation();
                //     ImGui::Text("RelPosVectorFwd:%8.3f%8.3f%8.3f", rel_pos.x, rel_pos.y, rel_pos.z);
                //     ImGui::Text("RelRotVectorFwd:%8.3f%8.3f%8.3f", rel_rot.x, rel_rot.y, rel_rot.z);
                // }

                // if (const Trajectory* const bwd_minus_traj = switch_->getBwdMinusTraj())
                // {
                //     ImGui::Text("bwdMinusTraj: %s", bwd_minus_traj->getName().toStdString().c_str());
                //     ImGui::Text("state_bwd: %d", switch_->getStateBwd());
                // }

                // if (const Trajectory* const fwd_minus_traj = switch_->getFwdMinusTraj())
                // {
                //     ImGui::Text("fwdMinusTraj: %s", fwd_minus_traj->getName().toStdString().c_str());
                //     ImGui::Text("state_fwd: %d", switch_->getStateFwd());
                // }

                ImGui::TreePop();
            }
        }
    }

    ImGui::End();
}

void EditorGui::show_selected_object_properties() const
{
    if (auto matrix_transform = *editor_params->selected_object)
    {
        ObjectProperties properties;
        if (matrix_transform->getValue("properties", properties))
        {
            ImGui::Begin("Properties", nullptr, window_flags);

            ImGui::Text("name: %s", properties.name.c_str());

            ImGui::Text("translation:");
            if (ImGui::SliderFloat("x##translation", &properties.translation.x, -100.0f, 100.0f))
            {
                update_selected_object_matrix(properties, matrix_transform);
            }
            if (ImGui::SliderFloat("y##translation", &properties.translation.y, -100.0f, 10000.0f))
            {
                update_selected_object_matrix(properties, matrix_transform);
            }
            if (ImGui::SliderFloat("z##translation", &properties.translation.z, -100.0f, 100.0f))
            {
                update_selected_object_matrix(properties, matrix_transform);
            }

            ImGui::Text("rotation:");
            if (ImGui::SliderFloat("x##rotation", &properties.rotation.x, -180.0f, 180.0f))
            {
                update_selected_object_matrix(properties, matrix_transform);
            }
            if (ImGui::SliderFloat("y##rotation", &properties.rotation.y, -180.0f, 180.0f))
            {
                update_selected_object_matrix(properties, matrix_transform);
            }
            if (ImGui::SliderFloat("z##rotation", &properties.rotation.z, -180.0f, 180.0f))
            {
                update_selected_object_matrix(properties, matrix_transform);
            }

            ImGui::End();
        }
    }
}

static void update_selected_object_matrix(
    const ObjectProperties& properties,
    vsg::ref_ptr<vsg::MatrixTransform> matrix_transform
)
{
    vsg::vec3 rotation = properties.rotation;

    rotation.x = -vsg::radians(rotation.x);
    rotation.y = -vsg::radians(rotation.y);
    rotation.z = -vsg::radians(rotation.z);

    const vsg::mat4 rotate_x = vsg::rotate(rotation.x, vsg::vec3(1.0f, 0.0f, 0.0f));
    const vsg::mat4 rotate_y = vsg::rotate(rotation.y, vsg::vec3(0.0f, 1.0f, 0.0f));
    const vsg::mat4 rotate_z = vsg::rotate(rotation.z, vsg::vec3(0.0f, 0.0f, 1.0f));
    const vsg::mat4 translate = vsg::translate(properties.translation);

    matrix_transform->matrix = translate * rotate_z * rotate_y * rotate_x;
    matrix_transform->setValue("properties", properties);
}
