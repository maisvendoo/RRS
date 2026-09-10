#define IMGUI_DEFINE_MATH_OPERATORS

#include "editor/EditorGui.h"

#include "editor/Camera.h"
#include "editor/EditorContext.h"
#include "editor/EventHandler.h"
#include "editor/KeyBindings.h"
#include "editor/MeasureTool.h"
#include "editor/ScreenProjector.h"
#include "editor/SplineTool.h"
#include "editor/Route.h"
#include "editor/RouteObject.h"
#include "editor/TrackFurniture.h"
#include "editor/TrackProfile.h"
#include "editor/commands/AddObject.h"
#include "editor/commands/CommandList.h"
#include "editor/commands/DeleteObjects.h"
#include "editor/commands/PastePrefab.h"
#include "editor/commands/ReplaceLabel.h"
#include "editor/commands/ResetScale.h"
#include "editor/commands/RotateObjects.h"
#include "editor/commands/ScaleObjects.h"
#include "editor/commands/TranslateObjects.h"
#include "editor/settings/CameraSettings.h"
#include "editor/settings/GuiSettings.h"
#include "editor/states/EditorState.h"

#include <Journal.h>
#include <filesystem.h>

#include <CfgReader.h>

#include <topology-defines.h>
#include <topology.h>
#include <switch.h>
#include <track.h>
#include <trajectory.h>
#include <vec3.h>

#include <ImGuiFileDialog.h>
#include <vsg/app/Viewer.h>
#include <vsg/core/Mask.h>
#include <vsg/maths/common.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsgImGui/imgui.h>
#include <vsgImGui/imgui_internal.h>

#include <algorithm>
#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>
#include <string>
#include <vector>

static bool drag_double3(const char* label, double* data, float speed = 1.0f,
    const double* min = nullptr, const double* max = nullptr,
    ImGuiSliderFlags flags = 0)
{
    return ImGui::DragScalarN(label, ImGuiDataType_Double, data, 3,
        speed, min, max, "%.3f", flags);
}

/// Геттер имён слоёв для ImGui::Combo
static const char* layer_name_getter(void* data, int index)
{
    const auto* const names =
        static_cast<const std::vector<std::string>*>(data);

    if (index < 0 ||
        static_cast<std::size_t>(index) >= names->size())
    {
        return nullptr;
    }

    return (*names)[static_cast<std::size_t>(index)].c_str();
}

EditorGui::EditorGui(EditorContext& context,
                     const gui_settings_t& gui_settings_)
    : context_(context)
    , gui_settings(gui_settings_)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    const FileSystem& fs = FileSystem::getInstance();

    const char* const font_name = "JetBrainsMono-Regular.ttf";
    const std::string font_path = fs.combinePath(fs.getFontsDir(), font_name);

    // Глифы кириллицы — иначе русские подписи не отображаются
    io.Fonts->AddFontFromFileTTF(font_path.c_str(), gui_settings.font_size,
        nullptr, io.Fonts->GetGlyphRangesCyrillic());

    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 6.0f;
    style.FrameBorderSize = 1.0f;
    style.WindowRounding = 6.0f;
    style.ScrollbarSize = 16.0f;
    style.GrabMinSize = 16.0f;
    style.GrabRounding = 6.0f;

    viewport = ImGui::GetMainViewport();

    new_route_parent_dir = fs.getRouteRootDir();

    // Избранное Model browser (editor-settings.xml, секция Favorites)
    load_favorites();
}

void EditorGui::record(vsg::CommandBuffer&) const
{
    draw_menu_bar();
    draw_status_bar();

    draw_open_route_file_dialog();
    draw_new_route_dir_dialog();
    draw_new_route_popup();
    draw_invalid_route_popup();

    if (gui_settings.show_objects_ref)
    {
        draw_objects_ref_window();
    }

    if (gui_settings.show_route_map)
    {
        draw_route_map_window();
    }

    if (gui_settings.show_stations_conf)
    {
        draw_stations_window();
    }

    if (gui_settings.show_topology)
    {
        draw_topology_window();
    }

    if (gui_settings.show_selected_objects_properties)
    {
        draw_selected_objects_window();
    }

    if (gui_settings.show_commands)
    {
        draw_commands_window();
    }

    // Инспектор выбранного участка пути (режим "Пути")
    draw_track_window();

    // Окна инструментов (промт Этапа 1)
    if (gui_settings.show_key_bindings)
    {
        draw_key_bindings_window();
    }

    if (gui_settings.show_validator)
    {
        draw_validator_window();
    }

    // Маркер режима установки объектов: круг на земле + подпись
    if (context_.place_object_mode && context_.place_cursor_valid)
    {
        ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
        vsg::dvec2 center;

        if (editor2::project_world_to_screen(context_,
                                             context_.place_cursor_world,
                                             center))
        {
            const ImVec2 c(static_cast<float>(center.x),
                           static_cast<float>(center.y));

            draw_list->AddCircle(c, 22.0f, IM_COL32(90, 220, 110, 230), 24, 2.5f);
            draw_list->AddCircleFilled(c, 3.0f, IM_COL32(90, 220, 110, 230));

            draw_list->AddText(ImVec2(c.x + 26.0f, c.y - 8.0f),
                               IM_COL32(120, 255, 150, 255),
                               context_.place_object_label.c_str());

            // Стрелка текущего поворота
            const double a = context_.place_object_rotation_deg *
                             3.14159265358979 / 180.0;
            const ImVec2 tip(c.x + static_cast<float>(std::cos(a)) * 18.0f,
                             c.y + static_cast<float>(std::sin(a)) * 18.0f);
            draw_list->AddLine(c, tip, IM_COL32(90, 220, 110, 230), 2.0f);
        }
    }

    // Мини-карта (TSRE MapWindow): подложка + схема маршрута
    if (gui_settings.show_minimap)
    {
        if (!minimap)
        {
            minimap = std::make_unique<MiniMap>(const_cast<EditorContext&>(context_));
            minimap->setRouteAnchor(context_.route_latitude,
                                    context_.route_longitude);
        }

        minimap->draw();
    }

    // Окна браузера моделей, массовых операций и слоёв
    if (gui_settings.show_model_browser)
    {
        draw_model_browser_window();
    }

    if (gui_settings.show_mass_edit)
    {
        draw_mass_edit_window();
    }

    if (gui_settings.show_layers)
    {
        draw_layers_window();
    }

    // Окно префабов (сохранение/вставка наборов объектов)
    if (gui_settings.show_prefabs)
    {
        draw_prefabs_window();
    }

    // Рамка выделения и оверлей измерений - поверх окон
    draw_selection_rect();

    if (context_.measure_tool && context_.measure_tool->is_active())
    {
        context_.measure_tool->draw_overlay();
    }

    // Оверлей построения нового пути (клавиша N): окно авто,
    // пока активен режим - флаг GuiSettings не нужен
    if (context_.spline_tool && context_.spline_tool->is_active())
    {
        context_.spline_tool->draw_overlay();
    }

    draw_progress_window();
}

void EditorGui::draw_menu_bar() const
{
    if (!ImGui::BeginMainMenuBar())
    {
        return;
    }

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("New route..."))
        {
            ImGui::OpenPopup("NewRoute");
        }

        if (ImGui::MenuItem("Open route..."))
        {
            IGFD::FileDialogConfig config;
            config.path = FileSystem::getInstance().getRouteRootDir();
            ImGuiFileDialog::Instance()->OpenDialog("OpenRouteKey",
                "Open route", nullptr, config);
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Save route", "Ctrl+S", false,
            context_.state == EditorState::EDIT_ROUTE))
        {
            context_.event_handler->save_route();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Quit", "Esc"))
        {
            if (context_.viewer)
            {
                context_.viewer->close();
            }
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View"))
    {
        ImGui::MenuItem("objects.ref", nullptr,
            &context_.gui_settings.show_objects_ref);
        ImGui::MenuItem("route1.map", nullptr,
            &context_.gui_settings.show_route_map);
        ImGui::MenuItem("stations.conf", nullptr,
            &context_.gui_settings.show_stations_conf);
        ImGui::MenuItem("Topology", nullptr,
            &context_.gui_settings.show_topology);
        ImGui::MenuItem("Selected objects", nullptr,
            &context_.gui_settings.show_selected_objects_properties);
        ImGui::MenuItem("Commands", nullptr,
            &context_.gui_settings.show_commands);
        ImGui::MenuItem("Key bindings", nullptr,
            &context_.gui_settings.show_key_bindings);
        ImGui::MenuItem("Validator", nullptr,
            &context_.gui_settings.show_validator);
        ImGui::MenuItem(u8"Карта", nullptr,
            &context_.gui_settings.show_minimap);
        ImGui::MenuItem("Model browser", nullptr,
            &context_.gui_settings.show_model_browser);
        ImGui::MenuItem("Mass edit", nullptr,
            &context_.gui_settings.show_mass_edit);
        ImGui::MenuItem("Layers", nullptr,
            &context_.gui_settings.show_layers);
        ImGui::MenuItem("Prefabs", nullptr,
            &context_.gui_settings.show_prefabs);

        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void EditorGui::draw_status_bar() const
{
    if (!ImGui::BeginViewportSideBar("StatusBar", viewport, ImGuiDir_Down,
        ImGui::GetFrameHeight(), ImGuiWindowFlags_NoScrollbar))
    {
        return;
    }

    std::string text;

    if (context_.route_dir.empty())
    {
        text = "No route | ";
    }
    else
    {
        text = context_.route_dir + " | ";
    }

    text += "objects: " +
        std::to_string(context_.static_objects_count.load()) + " / " +
        std::to_string(context_.total_static_objects_count.load());

    if (context_.topology_loaded)
    {
        text += " | topology: " +
            std::to_string(context_.topology_objects_count.load()) + " / " +
            std::to_string(context_.total_topology_objects_count.load());
    }

    text += " | speed: " +
        std::to_string(static_cast<int>(context_.camera_settings.move_speed));

    if (context_.trajectory_mode)
    {
        text += " | PATH MODE (P)";
    }

    if (context_.selected_trajectory != nullptr)
    {
        text += " | path: " + context_.selected_trajectory_name;
    }

    if (!context_.status.empty())
    {
        text += " | " + context_.status;
    }

    ImGui::TextUnformatted(text.c_str());

    ImGui::End();
}

void EditorGui::draw_open_route_file_dialog() const
{
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    if (!ImGuiFileDialog::Instance()->Display("OpenRouteKey",
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse))
    {
        return;
    }

    if (ImGuiFileDialog::Instance()->IsOk())
    {
        const std::string route_dir =
            ImGuiFileDialog::Instance()->GetCurrentPath();

        // Обязателен только topology (импорты OSM/GPX создают его);
        // models/textures/objects.ref опциональны - их можно добавить
        // позже (в т.ч. скопировав из другого маршрута)
        if (!std::filesystem::exists(route_dir + "/topology"))
        {
            ImGuiFileDialog::Instance()->Close();
            ImGui::OpenPopup("InvalidRoute");
        }
        else
        {
            ImGuiFileDialog::Instance()->Close();

            // Загружаем маршрут в сцену в начале следующего кадра
            context_.route_dir = route_dir;
            context_.state = EditorState::LOAD_ROUTE;
        }
    }
    else
    {
        ImGuiFileDialog::Instance()->Close();
    }
}

void EditorGui::draw_new_route_dir_dialog() const
{
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    if (!ImGuiFileDialog::Instance()->Display("NewRouteDirKey",
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse))
    {
        return;
    }

    if (ImGuiFileDialog::Instance()->IsOk())
    {
        new_route_parent_dir =
            ImGuiFileDialog::Instance()->GetCurrentPath();
    }

    ImGuiFileDialog::Instance()->Close();
}

void EditorGui::draw_new_route_popup() const
{
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (!ImGui::BeginPopupModal("NewRoute", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }

    ImGui::Text("Create new route");
    ImGui::Separator();

    ImGui::InputText("Name", new_route_name, sizeof(new_route_name));

    ImGui::Text("Location: %s", new_route_parent_dir.c_str());
    ImGui::SameLine();

    if (ImGui::Button("Choose..."))
    {
        IGFD::FileDialogConfig config;
        config.path = new_route_parent_dir;
        ImGuiFileDialog::Instance()->OpenDialog("NewRouteDirKey",
            "Choose location", nullptr, config);
    }

    ImGui::Separator();

    if (ImGui::Button("Create", ImVec2(120.0f, 0)))
    {
        const std::string name(new_route_name);

        if (!name.empty())
        {
            if (create_route_structure(new_route_parent_dir, name))
            {
                new_route_name[0] = '\0';
                ImGui::CloseCurrentPopup();
            }
            else
            {
                context_.status = "Failed to create route structure";
            }
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(120.0f, 0)))
    {
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void EditorGui::draw_invalid_route_popup() const
{
    if (!ImGui::BeginPopupModal("InvalidRoute", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
    {
        return;
    }

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

void EditorGui::draw_objects_ref_window() const
{
    if (!ImGui::Begin("objects.ref", nullptr))
    {
        ImGui::End();
        return;
    }

    if (context_.objects_ref.empty())
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

            ImGui::PushID(label.c_str());

            if (ImGui::Button("+"))
            {
                add_object(label);
            }

            ImGui::SameLine();
            ImGui::TextUnformatted(label.c_str());

            ImGui::PopID();

            ImGui::TableNextColumn();
            ImGui::Text("%s", ref.relative_path.c_str());
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void EditorGui::draw_route_map_window() const
{
    if (!ImGui::Begin("route1.map", nullptr))
    {
        ImGui::End();
        return;
    }

    if (context_.route_map.empty())
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

void EditorGui::draw_stations_window() const
{
    if (!ImGui::Begin("stations.conf", nullptr))
    {
        ImGui::End();
        return;
    }

    if (context_.stations_conf.empty())
    {
        ImGui::Text("There is no stations yet");
        ImGui::End();
        return;
    }

    if (ImGui::BeginTable("stations_conf_table", 4,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg))
    {
        for (const auto& [label, translation] : context_.stations_conf)
        {
            constexpr const char* number_format = "%10.3f";

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            ImGui::PushID(label.c_str());

            if (ImGui::Button("Go"))
            {
                const auto look_at = context_.camera->get_look_at();

                look_at->eye = translation + vsg::dvec3(0.0, 0.0, 50.0);
                look_at->center = look_at->eye +
                    context_.event_handler->get_front();
            }

            ImGui::SameLine();
            ImGui::TextUnformatted(label.c_str());

            ImGui::PopID();

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

void EditorGui::draw_topology_window() const
{
    if (!ImGui::Begin("Topology", nullptr))
    {
        ImGui::End();
        return;
    }

    if (!context_.topology_loaded.load())
    {
        ImGui::Text("Topology is not loaded yet");
        ImGui::End();
        return;
    }

    // Режим выбора траекторий кликом мыши (клавиша P)
    ImGui::Checkbox("Path mode (P)", &context_.trajectory_mode);
    ImGui::Separator();

    /// Строка списка траекторий (копия под мьютексом)
    struct TrajectoryRow
    {
        std::string name;
        double length = 0.0;
        vsg::dvec3 begin_point = {0.0, 0.0, 0.0};
        vsg::dvec3 direction = {0.0, 1.0, 0.0};
        std::size_t switch_count = 0;
    };

    std::vector<TrajectoryRow> rows;

    {
        std::lock_guard<std::mutex> lock_guard(context_.topology_mutex);

        if (!context_.topology)
        {
            ImGui::Text("Topology is not loaded yet");
            ImGui::End();
            return;
        }

        const traj_list_t* const traj_list =
            context_.topology->getTrajectoriesList();

        const sw_list_t* const connectors =
            context_.topology->getConnectorsList();

        for (const Trajectory* trajectory : *traj_list)
        {
            TrajectoryRow row;
            row.name = trajectory->getName().toStdString();
            row.length = trajectory->getLength();

            const auto& tracks = trajectory->getTracks();

            if (!tracks.empty())
            {
                const dvec3& begin = tracks.front().begin_point;
                row.begin_point = vsg::dvec3{begin.x, begin.y, begin.z};

                // Направление вдоль пути в плане (для телепорта камеры)
                const dvec3& orth = tracks.front().orth;
                const vsg::dvec3 plan_direction =
                    vsg::dvec3{orth.x, orth.y, 0.0};

                if (vsg::length(plan_direction) > 1.0e-6)
                {
                    row.direction = vsg::normalize(plan_direction);
                }
            }

            // Количество стрелок, подключённых к траектории
            for (const Switch* connector : *connectors)
            {
                for (const Trajectory* connected : connector->trajectories)
                {
                    if (connected == trajectory)
                    {
                        ++row.switch_count;
                    }
                }
            }

            rows.emplace_back(std::move(row));
        }
    }

    if (ImGui::BeginTable("topology_table", 6,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
    {
        ImGui::TableSetupColumn("Go");
        ImGui::TableSetupColumn("Trajectory");
        ImGui::TableSetupColumn("Length, m");
        ImGui::TableSetupColumn("Grade, ppm");
        ImGui::TableSetupColumn("Limit, km/h");
        ImGui::TableSetupColumn("Switches");
        ImGui::TableHeadersRow();

        for (const TrajectoryRow& row : rows)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            ImGui::PushID(row.name.c_str());

            if (ImGui::Button("Go"))
            {
                // Телепорт камеры к началу траектории
                // (по образцу окна stations)
                const auto look_at = context_.camera->get_look_at();

                look_at->eye = row.begin_point + vsg::dvec3(0.0, 0.0, 50.0);
                look_at->center = look_at->eye + row.direction * 100.0;
            }

            ImGui::TableNextColumn();

            // Клик по строке - выбор траектории (как в режиме "Пути")
            const bool is_selected =
                context_.selected_trajectory_name == row.name;

            if (ImGui::Selectable(row.name.c_str(), is_selected,
                ImGuiSelectableFlags_SpanAllColumns))
            {
                if (context_.route)
                {
                    context_.route->select_trajectory(row.name);
                }
            }

            ImGui::TableNextColumn();
            ImGui::Text("%10.3f", row.length);

            ImGui::TableNextColumn();

            const auto profile_it = context_.track_profiles.find(row.name);

            if (profile_it != context_.track_profiles.end())
            {
                ImGui::Text("%6.1f", profile_it->second.grade_promille);

                ImGui::TableNextColumn();
                ImGui::Text("%6.1f", profile_it->second.speed_limit);
            }
            else
            {
                ImGui::TextUnformatted("-");

                ImGui::TableNextColumn();
                ImGui::TextUnformatted("-");
            }

            ImGui::TableNextColumn();

            // Клик по числу стрелок - выбор траектории
            // (как клик по имени в колонке Trajectory)
            char switches_text[32];
            std::snprintf(switches_text, sizeof(switches_text), "%zu",
                row.switch_count);

            if (ImGui::Selectable(switches_text, is_selected))
            {
                if (context_.route)
                {
                    context_.route->select_trajectory(row.name);
                }
            }

            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void EditorGui::draw_selected_objects_window() const
{
    const RouteObjects& selected_objects = context_.selected_objects;

    if (selected_objects.empty())
    {
        return;
    }

    if (!ImGui::Begin("Selected objects", nullptr))
    {
        ImGui::End();
        return;
    }

    // Точное преобразование (по мотивам TSRE TransformWorldObjDialog):
    // численный ввод позиции/поворота/масштаба первого выбранного объекта
    if (ImGui::CollapsingHeader(u8"Точное преобразование"))
    {
        auto object = selected_objects.front();

        vsg::dvec3 translation = object->get_translation();
        vsg::dvec3 rotation = object->get_rotation_deg();
        vsg::dvec3 scale = object->get_scale();

        float pos[3] = {static_cast<float>(translation.x),
                        static_cast<float>(translation.y),
                        static_cast<float>(translation.z)};
        float rot[3] = {static_cast<float>(rotation.x),
                        static_cast<float>(rotation.y),
                        static_cast<float>(rotation.z)};
        float scl[3] = {static_cast<float>(scale.x),
                        static_cast<float>(scale.y),
                        static_cast<float>(scale.z)};

        bool changed = false;

        changed |= ImGui::InputFloat3(u8"Позиция X/Y/Z, м", pos, "%.2f",
                                      ImGuiInputTextFlags_EnterReturnsTrue);
        changed |= ImGui::InputFloat3(u8"Поворот X/Y/Z, град", rot, "%.1f",
                                      ImGuiInputTextFlags_EnterReturnsTrue);
        changed |= ImGui::InputFloat3(u8"Масштаб X/Y/Z", scl, "%.3f",
                                      ImGuiInputTextFlags_EnterReturnsTrue);

        if (changed)
        {
            object->set_translation(vsg::dvec3(pos[0], pos[1], pos[2]));
            object->set_rotation_deg(vsg::dvec3(rot[0], rot[1], rot[2]));
            object->set_scale(vsg::dvec3(scl[0], scl[1], scl[2]));
        }

        if (ImGui::Button(u8"Сбросить масштаб"))
        {
            object->set_scale(vsg::dvec3(1.0, 1.0, 1.0));
        }

        ImGui::SameLine();

        if (ImGui::Button(u8"На уровень земли (Z=0)"))
        {
            translation = object->get_translation();
            translation.z = 0.0;
            object->set_translation(translation);
        }
    }

    std::size_t i = 0;

    for (const auto& object : selected_objects)
    {
        ImGui::PushID(object.get());

        ImGui::Text("label: %s", object->label.c_str());

        ObjectDragState& drag_state = get_drag_state(object.get());

        handle_translation_drag(i, object, drag_state);
        handle_rotation_drag(i, object, drag_state);
        handle_scale_drag(i, object, drag_state);

        ImGui::PopID();

        ++i;
    }

    ImGui::End();
}

void EditorGui::draw_commands_window() const
{
    if (!ImGui::Begin("Commands", nullptr))
    {
        ImGui::End();
        return;
    }

    auto active = context_.commands.get_active();
    auto curr = context_.commands.get_tail();

    while (curr)
    {
        if (curr == active)
        {
            ImGui::TextColored(ImVec4{0.2f, 1.0f, 0.3f, 1.0f}, "%s",
                curr->command->get_description());
        }
        else
        {
            ImGui::Text("%s", curr->command->get_description());
        }

        ImGui::Separator();

        curr = curr->prev;
    }

    ImGui::End();
}

void EditorGui::draw_track_window() const
{
    // Инспектор показывается только при выбранной траектории
    const Trajectory* const trajectory = context_.selected_trajectory;

    if (trajectory == nullptr)
    {
        return;
    }

    if (!ImGui::Begin("Путь", nullptr))
    {
        ImGui::End();
        return;
    }

    const std::string& name = context_.selected_trajectory_name;
    const double length = trajectory->getLength();

    // Профиль участка (создаётся с настройками по умолчанию)
    TrackProfile& profile = context_.track_profiles[name];

    ImGui::Text("Траектория: %s", name.c_str());
    ImGui::Text("Длина: %.1f м", length);
    ImGui::Separator();

    // Пресеты верхнего строения пути
    if (ImGui::Button("Главный путь РЖД"))
    {
        profile.rail_variant = "Р65";
        profile.sleeper_variant = "жб";
        profile.ballast_variant = "щебень";
    }

    ImGui::SameLine();

    if (ImGui::Button("Станционный путь"))
    {
        profile.rail_variant = "Р50";
        profile.sleeper_variant = "дерево";
        profile.ballast_variant = "щебень";
    }

    // Комбобоксы вариантов верхнего строения пути
    static const char* const rail_variants[] = {"Р50", "Р65", "Р75"};
    static const char* const sleeper_variants[] = {"дерево", "жб"};
    static const char* const ballast_variants[] = {"щебень", "песок"};

    const auto draw_variant_combo =
        [](const char* label, const char* const variants[],
           int variants_count, std::string& value) -> void
    {
        int current = 0;

        for (int i = 0; i < variants_count; ++i)
        {
            if (value == variants[i])
            {
                current = i;
                break;
            }
        }

        if (ImGui::Combo(label, &current, variants, variants_count))
        {
            value = variants[current];
        }
    };

    draw_variant_combo("Рельсы", rail_variants,
        IM_ARRAYSIZE(rail_variants), profile.rail_variant);

    draw_variant_combo("Шпалы", sleeper_variants,
        IM_ARRAYSIZE(sleeper_variants), profile.sleeper_variant);

    draw_variant_combo("Щебень", ballast_variants,
        IM_ARRAYSIZE(ballast_variants), profile.ballast_variant);

    // Скрытие отдельных мешей участка
    ImGui::Checkbox("Скрыть рельсы", &profile.rail_hidden);
    ImGui::Checkbox("Скрыть шпалы", &profile.sleeper_hidden);
    ImGui::Checkbox("Скрыть балласт", &profile.ballast_hidden);

    // Отметка стыков рельсов жёлтыми поперечинами каждые 25 м
    // (промт п.12, упрощённо; параметр геометрии TrackMesh)
    ImGui::Checkbox("Отмечать стыки (каждые 25 м)", &profile.mark_joints);

    ImGui::Separator();

    // Продольный уклон: поле в тысячных и в десятичных долях,
    // синхронизированные между собой (grade_promille = grade_decimal * 1000)
    bool grade_changed = false;

    double promille = profile.grade_promille;

    if (ImGui::InputDouble("Уклон, тысячные", &promille, 0.1, 1.0, "%.2f"))
    {
        profile.grade_promille = promille;
        profile.grade_decimal = promille / 1000.0;
        grade_changed = true;
    }

    // Колёсико мыши над полем: Shift - мелкий шаг 0.01 тысячных
    if (ImGui::IsItemHovered() && ImGui::GetIO().MouseWheel != 0.0f)
    {
        const double wheel_step = ImGui::GetIO().KeyShift ? 0.01 : 0.1;

        profile.grade_promille += static_cast<double>(
            ImGui::GetIO().MouseWheel) * wheel_step;

        profile.grade_promille = std::clamp(profile.grade_promille,
            -40.0, 40.0);

        profile.grade_decimal = profile.grade_promille / 1000.0;
        grade_changed = true;
    }

    double decimal = profile.grade_decimal;

    if (ImGui::InputDouble("Уклон, доли", &decimal, 0.0001, 0.001, "%.5f"))
    {
        profile.grade_decimal = decimal;
        profile.grade_promille = decimal * 1000.0;
        grade_changed = true;
    }

    if (ImGui::IsItemHovered() && ImGui::GetIO().MouseWheel != 0.0f)
    {
        const double wheel_step = ImGui::GetIO().KeyShift ? 0.00001 : 0.0001;

        profile.grade_decimal += static_cast<double>(
            ImGui::GetIO().MouseWheel) * wheel_step;

        profile.grade_promille = profile.grade_decimal * 1000.0;
        grade_changed = true;
    }

    if (grade_changed)
    {
        // Изменение уклона пересчитывает профиль участка:
        // высота конца из уклона и длины
        profile.update_end_elevation(length);
    }

    if (ImGui::InputDouble("Высота начала, м", &profile.begin_elevation,
        0.1, 1.0, "%.3f"))
    {
        profile.update_end_elevation(length);
    }

    if (ImGui::InputDouble("Высота конца, м", &profile.end_elevation,
        0.1, 1.0, "%.3f"))
    {
        // Изменение высоты конца пересчитывает уклон
        profile.update_grade(length);
    }

    // Наследование высоты при продолжении пути: соседним траекториям
    // (по стрелкам) передаётся высота конца участка как высота начала
    if (ImGui::Button("Высоту конца -> соседям"))
    {
        std::lock_guard<std::mutex> lock_guard(context_.topology_mutex);

        if (context_.topology)
        {
            const sw_list_t* const connectors =
                context_.topology->getConnectorsList();

            for (const Switch* connector : *connectors)
            {
                bool has_selected = false;

                for (const Trajectory* connected : connector->trajectories)
                {
                    if (connected == trajectory)
                    {
                        has_selected = true;
                        break;
                    }
                }

                if (!has_selected)
                {
                    continue;
                }

                for (const Trajectory* connected : connector->trajectories)
                {
                    if (connected == nullptr || connected == trajectory)
                    {
                        continue;
                    }

                    context_.track_profiles[
                        connected->getName().toStdString()]
                        .begin_elevation = profile.end_elevation;
                }
            }

            context_.status = "End elevation is passed to the neighbors";
        }
    }

    ImGui::Separator();

    // Вертикальная переходная кривая между соседними уклонами
    ImGui::Checkbox("Вертикальная кривая: авто", &profile.vertical_curve_auto);

    if (profile.vertical_curve_auto)
    {
        // Радиус по лимиту скорости:
        // 60 км/ч -> 2000 м, 80 -> 6000, 120 -> 15000, 160 -> 25000
        ImGui::Text("Радиус (авто): %.0f м",
            profile.auto_vertical_curve_radius());
    }
    else
    {
        if (ImGui::InputDouble("Радиус кривой, м",
            &profile.vertical_curve_radius, 100.0, 1000.0, "%.0f"))
        {
            profile.vertical_curve_radius = std::max(
                profile.vertical_curve_radius, 100.0);
        }
    }

    ImGui::InputDouble("Длина переходной, м", &profile.transition_length,
        10.0, 100.0, "%.1f");

    ImGui::InputDouble("Лимит скорости, км/ч", &profile.speed_limit,
        5.0, 20.0, "%.0f");

    ImGui::Separator();

    // Генерация обвеса пути (промт п.9-11, упрощённо)
    draw_track_generation_section(name);

    ImGui::Separator();

    // Параметрическая стройка растягиванием (ТЗ п.6/61)
    draw_build_section(name);

    ImGui::Separator();

    // Разложить выбранный объект вдоль пути (промт п.7/29)
    draw_follow_path_section(name);

    ImGui::Separator();

    if (ImGui::Button("Сохранить track-edit.conf"))
    {
        save_track_edit_conf();
    }

    ImGui::End();
}

void EditorGui::draw_build_section(const std::string& trajectory_name) const
{
    if (!ImGui::CollapsingHeader(u8"Стройка растягиванием"))
    {
        return;
    }

    if (trajectory_name.empty())
    {
        ImGui::TextDisabled(u8"Сначала выберите траекторию (клавиша P)");
        return;
    }

    auto& context = const_cast<EditorContext&>(context_);

    static const char* const modes[] =
    {
        u8"Выключено",
        u8"Опоры КС",
        u8"Платформа",
        u8"Насыпь",
        u8"Выемка",
        u8"Кювет"
    };

    int mode_index = static_cast<int>(context.build_mode);

    if (ImGui::Combo(u8"Что строить", &mode_index, modes, IM_ARRAYSIZE(modes)))
    {
        context.build_mode = static_cast<EditorContext::BuildMode>(mode_index);
    }

    // Параметры выбранного режима
    switch (context.build_mode)
    {
        case EditorContext::BuildMode::Catenary:
        {
            if (ImGui::InputDouble(u8"Интервал опор, м",
                                   &context.build_step_m, 5.0, 10.0, "%.0f"))
            {
                context.build_step_m = std::clamp(context.build_step_m, 10.0, 200.0);
            }
            break;
        }

        case EditorContext::BuildMode::Platform:
        {
            ImGui::InputDouble(u8"Ширина, м", &context.build_platform_width_m,
                0.5, 1.0, "%.1f");
            ImGui::InputDouble(u8"Высота, м", &context.build_platform_height_m,
                0.05, 0.1, "%.2f");
            ImGui::Checkbox(u8"Справа от пути", &context.build_right_side);
            break;
        }

        case EditorContext::BuildMode::Embankment:
        {
            ImGui::InputDouble(u8"Высота насыпи, м", &context.build_embank_height_m,
                0.5, 1.0, "%.1f");
            ImGui::InputDouble(u8"Ширина бермы, м", &context.build_embank_shoulder_m,
                0.5, 1.0, "%.1f");
            break;
        }

        case EditorContext::BuildMode::Cutting:
        {
            ImGui::InputDouble(u8"Глубина, м", &context.build_cut_depth_m,
                0.5, 1.0, "%.1f");
            ImGui::InputDouble(u8"Ширина, м", &context.build_cut_width_m,
                1.0, 5.0, "%.1f");
            break;
        }

        case EditorContext::BuildMode::Ditch:
        {
            ImGui::Checkbox(u8"Справа от пути", &context.build_right_side);
            ImGui::InputDouble(u8"Ширина, м", &context.build_ditch_width_m,
                0.5, 1.0, "%.1f");
            ImGui::InputDouble(u8"Глубина, м", &context.build_ditch_depth_m,
                0.1, 0.5, "%.2f");
            break;
        }

        default:
        {
            break;
        }
    }

    if (context.build_mode != EditorContext::BuildMode::None)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f),
            u8"ЛКМ по пути — растянуть оранжевую полосу, отпустить — построить");
        ImGui::TextDisabled(u8"Путь: %s", trajectory_name.c_str());
    }
}

void EditorGui::draw_track_generation_section(
    const std::string& trajectory_name) const
{
    ImGui::TextUnformatted("Генерация");

    // Опоры контактовой сети каждые N метров
    if (ImGui::InputDouble("Опоры КС: шаг, м", &catenary_step,
        5.0, 10.0, "%.0f"))
    {
        catenary_step = std::clamp(catenary_step, 5.0, 500.0);
    }

    if (ImGui::Button("Опоры КС"))
    {
        // Генерация пишет записи в track-edit.conf (не в route1.map)
        if (TrackFurniture::generate_catenary_poles(context_,
            trajectory_name, catenary_step))
        {
            save_track_edit_conf();
        }
    }

    // Платформа вдоль пути
    ImGui::InputDouble("Платформа: длина, м", &platform_length,
        10.0, 50.0, "%.0f");
    ImGui::InputDouble("Платформа: ширина, м", &platform_width,
        0.5, 1.0, "%.1f");
    ImGui::InputDouble("Платформа: высота, м", &platform_height,
        0.05, 0.1, "%.2f");

    static const char* const platform_sides[] = {"справа", "слева"};

    ImGui::Combo("Платформа: сторона", &platform_side_index,
        platform_sides, IM_ARRAYSIZE(platform_sides));

    if (ImGui::Button("Платформа"))
    {
        if (TrackFurniture::generate_platform(context_, trajectory_name,
            platform_length, platform_width, platform_height,
            platform_side_index == 0))
        {
            save_track_edit_conf();
        }
    }

    // Километровые столбики каждые 1000 м
    if (ImGui::Button("Столбики км (каждые 1000 м)"))
    {
        if (TrackFurniture::generate_km_posts(context_, trajectory_name))
        {
            save_track_edit_conf();
        }
    }

    // Верхнее строение пути по профилю участка (рельсы + шпалы +
    // балласт на всю длину траектории, учитывая скрытие мешей)
    if (ImGui::Button("Сгенерировать путь"))
    {
        // Профиль участка: варианты рельсов/шпал и скрытые меши
        TrackProfile& profile = context_.track_profiles[trajectory_name];

        // Вся длина выбранной траектории (сэмпл ограничит её сам)
        const double length = (context_.selected_trajectory != nullptr)
            ? context_.selected_trajectory->getLength()
            : 0.0;

        if (TrackFurniture::generate_track_mesh(context_, trajectory_name,
            0.0, length, profile))
        {
            save_track_edit_conf();
        }
    }

    // Деревья вдоль пути: сторона, плотность и отступы от оси
    static const char* const trees_sides[] = {"справа", "слева"};

    ImGui::Combo("Деревья: сторона", &trees_side_index, trees_sides,
        IM_ARRAYSIZE(trees_sides));

    if (ImGui::InputDouble("Деревья: шт/км", &trees_per_km,
        10.0, 50.0, "%.0f"))
    {
        trees_per_km = std::clamp(trees_per_km, 1.0, 1000.0);
    }

    ImGui::InputDouble("Деревья: отступ от, м", &trees_offset_min,
        1.0, 5.0, "%.1f");
    ImGui::InputDouble("Деревья: отступ до, м", &trees_offset_max,
        1.0, 5.0, "%.1f");

    if (ImGui::Button("Деревья вдоль пути"))
    {
        if (TrackFurniture::generate_trees_along(context_, trajectory_name,
            trees_side_index == 0, trees_per_km, trees_offset_min,
            trees_offset_max))
        {
            save_track_edit_conf();
        }
    }

    // Вода у середины траектории: зеркало 200x200 м,
    // уровень = минимум высот профиля + 0.5 м
    if (ImGui::Button("Вода у середины траектории"))
    {
        const Trajectory* const trajectory = context_.selected_trajectory;

        if (trajectory != nullptr)
        {
            const TrackProfile& profile =
                context_.track_profiles[trajectory_name];

            const profile_point_t mid = trajectory->getPosition(
                trajectory->getLength() * 0.5, 1);

            const vsg::dvec3 center{mid.position.x, mid.position.y,
                mid.position.z};

            const double level = std::min(profile.begin_elevation,
                profile.end_elevation) + 0.5;

            if (TrackFurniture::generate_water(context_, center,
                200.0, 200.0, level))
            {
                save_track_edit_conf();
            }
        }
        else
        {
            context_.status = "Вода: траектория не выбрана";
        }
    }

    // Переезд на заданной координате вдоль траектории:
    // вместе с ним строится дорога перпендикулярно пути
    // (промт п.11, упрощённо)
    ImGui::InputDouble("Переезд: координата, м", &crossing_coord,
        10.0, 100.0, "%.1f");

    ImGui::InputDouble("Переезд: длина дороги, м", &crossing_road_length,
        10.0, 50.0, "%.0f");

    if (ImGui::Button("Переезд"))
    {
        if (TrackFurniture::generate_crossing(context_, trajectory_name,
            crossing_coord, crossing_road_length))
        {
            save_track_edit_conf();
        }
    }

    // Терраформинг вдоль пути: насыпь (трапеция с откосом 1:1.5)
    ImGui::Separator();

    ImGui::InputDouble("Насыпь: от, м", &embankment_from,
        10.0, 100.0, "%.1f");
    ImGui::InputDouble("Насыпь: до, м", &embankment_to,
        10.0, 100.0, "%.1f");
    ImGui::InputDouble("Насыпь: высота, м", &embankment_height,
        0.5, 1.0, "%.2f");
    ImGui::InputDouble("Насыпь: ширина верха, м", &embankment_shoulder,
        0.5, 1.0, "%.2f");

    if (ImGui::Button("Насыпь"))
    {
        if (TrackFurniture::generate_embankment(context_, trajectory_name,
            embankment_from, embankment_to, embankment_height,
            embankment_shoulder))
        {
            save_track_edit_conf();
        }
    }

    // Выемка: приподнятые стенки по краям пути
    ImGui::InputDouble("Выемка: от, м", &cutting_from,
        10.0, 100.0, "%.1f");
    ImGui::InputDouble("Выемка: до, м", &cutting_to,
        10.0, 100.0, "%.1f");
    ImGui::InputDouble("Выемка: глубина, м", &cutting_depth,
        0.5, 1.0, "%.2f");
    ImGui::InputDouble("Выемка: ширина, м", &cutting_width,
        1.0, 5.0, "%.1f");

    if (ImGui::Button("Выемка"))
    {
        if (TrackFurniture::generate_cutting(context_, trajectory_name,
            cutting_from, cutting_to, cutting_depth, cutting_width))
        {
            save_track_edit_conf();
        }
    }

    // Канава-кювет: узкий заглублённый бокс вдоль пути
    static const char* const ditch_sides[] = {"справа", "слева"};

    ImGui::Combo("Канава: сторона", &ditch_side_index, ditch_sides,
        IM_ARRAYSIZE(ditch_sides));

    ImGui::InputDouble("Канава: от, м", &ditch_from,
        10.0, 100.0, "%.1f");
    ImGui::InputDouble("Канава: до, м", &ditch_to,
        10.0, 100.0, "%.1f");
    ImGui::InputDouble("Канава: ширина, м", &ditch_width,
        0.1, 0.5, "%.2f");
    ImGui::InputDouble("Канава: глубина, м", &ditch_depth,
        0.1, 0.5, "%.2f");

    if (ImGui::Button("Канава"))
    {
        if (TrackFurniture::generate_ditch(context_, trajectory_name,
            ditch_from, ditch_to, ditch_side_index == 0, ditch_width,
            ditch_depth))
        {
            save_track_edit_conf();
        }
    }

    if (ImGui::Button("Убрать обвес траектории"))
    {
        if (TrackFurniture::remove_generated(context_, trajectory_name))
        {
            save_track_edit_conf();
        }
        else
        {
            context_.status = "У траектории нет обвеса";
        }
    }
}

void EditorGui::draw_follow_path_section(
    const std::string& trajectory_name) const
{
    ImGui::TextUnformatted("Разложить объект");

    // Метка объекта с валидацией по objects.ref
    ImGui::InputText("Метка объекта", follow_path_label,
        sizeof(follow_path_label));

    const std::string label(follow_path_label);

    const bool label_valid = !label.empty() &&
        context_.objects_ref.find(label) != context_.objects_ref.end();

    if (!label.empty())
    {
        if (label_valid)
        {
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f),
                "Метка найдена в objects.ref");
        }
        else
        {
            ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f),
                "Метки нет в objects.ref");
        }
    }

    // Метка первого выделенного объекта - в поле ввода
    if (ImGui::Button("Взять метку из выделения"))
    {
        if (!context_.selected_objects.empty())
        {
            const std::string selected_label =
                context_.selected_objects.front()->label;

            std::snprintf(follow_path_label, sizeof(follow_path_label),
                "%s", selected_label.c_str());
        }
        else
        {
            context_.status = "Разложить объект: нет выделения";
        }
    }

    if (ImGui::InputDouble("Шаг, м", &follow_path_step, 1.0, 10.0, "%.1f"))
    {
        follow_path_step = std::clamp(follow_path_step, 0.5, 100.0);
    }

    ImGui::InputDouble("Отступ от оси, м", &follow_path_offset,
        0.5, 5.0, "%.2f");

    ImGui::Checkbox("Поворот по касательной", &follow_path_rotate);

    ImGui::BeginDisabled(!label_valid);

    if (ImGui::Button("Разложить вдоль пути"))
    {
        // Сэмплы оси пути getPosition каждые step м (копия под
        // мьютексом топологии, как в TrackFurniture)
        struct FollowSample
        {
            vsg::dvec3 position = {0.0, 0.0, 0.0};
            vsg::dvec3 right = {0.0, 0.0, 0.0};
            vsg::dvec3 orth = {0.0, 1.0, 0.0};
        };

        std::vector<FollowSample> samples;

        {
            std::lock_guard<std::mutex> lock_guard(
                context_.topology_mutex);

            const Trajectory* const trajectory =
                context_.selected_trajectory;

            if (trajectory != nullptr)
            {
                const double length = trajectory->getLength();

                // Кап: не более 500 объектов за один вызов
                const double step = std::max(follow_path_step,
                    length / 500.0);

                for (double coord = 0.0; coord < length; coord += step)
                {
                    const profile_point_t point =
                        trajectory->getPosition(coord, 1);

                    FollowSample sample;
                    sample.position = vsg::dvec3{point.position.x,
                        point.position.y, point.position.z};
                    sample.right = vsg::dvec3{point.right.x,
                        point.right.y, point.right.z};
                    sample.orth = vsg::dvec3{point.orth.x,
                        point.orth.y, point.orth.z};

                    samples.push_back(sample);
                }
            }
        }

        if (samples.empty())
        {
            context_.status = "Разложить объект: траектория не выбрана";
        }
        else
        {
            const auto ref_it = context_.objects_ref.find(label);

            std::size_t created = 0;

            for (const FollowSample& sample : samples)
            {
                // Позиция: точка оси пути + правая сторона * отступ
                const vsg::dvec3 position = sample.position +
                    sample.right * follow_path_offset;

                // Поворот по касательной: локальная +Y вдоль пути
                // (тот же yaw, что у опор КС в TrackFurniture)
                vsg::dvec3 rotation_deg = {0.0, 0.0, 0.0};

                if (follow_path_rotate)
                {
                    rotation_deg.z = vsg::degrees(std::atan2(
                        -sample.orth.x, sample.orth.y));
                }

                const auto object = RouteObject::create(context_,
                    ref_it->second.paged_lod, label, position,
                    rotation_deg);

                context_.commands.push(new AddObject(context_, object),
                    true);

                ++created;
            }

            context_.status = "Разложено объектов: " +
                std::to_string(created) + " вдоль " + trajectory_name;
        }
    }

    ImGui::EndDisabled();
}

void EditorGui::save_track_edit_conf() const
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string track_edit_path = fs.combinePath(
        context_.route_dir, "track-edit.conf");

    // Профили участков + сгенерированный обвес + слои объектов
    if (save_track_edit_config(track_edit_path, context_))
    {
        context_.status = "Track profiles are saved: " + track_edit_path;
    }
    else
    {
        context_.status = "Failed to save track-edit.conf";
    }
}

void EditorGui::draw_progress_window() const
{
    if (context_.state != EditorState::EDIT_ROUTE)
    {
        return;
    }

    const auto total = context_.total_static_objects_count.load();
    const auto loaded = context_.static_objects_count.load();

    const bool topology_in_progress = context_.topology_loaded &&
        context_.topology_objects_count !=
            context_.total_topology_objects_count;

    if (loaded == total && !topology_in_progress)
    {
        return;
    }

    if (!ImGui::Begin("Progress", nullptr))
    {
        ImGui::End();
        return;
    }

    float fraction = 1.0f;
    if (total != 0)
    {
        fraction = static_cast<float>(loaded) / static_cast<float>(total);
    }

    char overlay[64];
    std::snprintf(overlay, sizeof(overlay), "%zu / %zu", loaded, total);

    ImGui::Text("Static objects:");
    ImGui::ProgressBar(fraction, ImVec2(200.0f, 20.0f), overlay);

    if (context_.topology_loaded)
    {
        const auto topology_total =
            context_.total_topology_objects_count.load();
        const auto topology_loaded =
            context_.topology_objects_count.load();

        fraction = 1.0f;
        if (topology_total != 0)
        {
            fraction = static_cast<float>(topology_loaded) /
                static_cast<float>(topology_total);
        }

        std::snprintf(overlay, sizeof(overlay), "%zu / %zu",
            topology_loaded, topology_total);

        ImGui::Text("Topology objects:");
        ImGui::ProgressBar(fraction, ImVec2(200.0f, 20.0f), overlay);
    }
    else
    {
        ImGui::Text("Topology not yet loaded");
    }

    ImGui::End();
}

void EditorGui::draw_key_bindings_window() const
{
    if (!ImGui::Begin("Key bindings", nullptr))
    {
        ImGui::End();
        return;
    }

    if (context_.key_binding_wait_action >= 0)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f),
            "Press a key to assign...");
    }
    else
    {
        ImGui::TextUnformatted("Click \"Assign\" and press a key");
    }

    ImGui::Spacing();

    if (ImGui::BeginTable("key_bindings_table", 3,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Action");
        ImGui::TableSetupColumn("Key");
        ImGui::TableSetupColumn("");
        ImGui::TableHeadersRow();

        for (int action = 0; action < TOTAL_ACTIONS; ++action)
        {
            const auto a = static_cast<Action>(action);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(to_c_string(a));
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(context_.key_bindings.to_string(a).c_str());
            ImGui::TableNextColumn();

            ImGui::PushID(action);

            if (ImGui::Button("Assign"))
            {
                // Следующее нажатие клавиши перехватит EventHandler
                context_.key_binding_wait_action = action;
            }

            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    ImGui::Spacing();

    if (ImGui::Button("Reset to defaults"))
    {
        context_.key_bindings = KeyBindings();
    }

    ImGui::End();
}

void EditorGui::draw_validator_window() const
{
    if (!ImGui::Begin("Validator", nullptr))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Run validation"))
    {
        validation_issues = Validator::validate(context_);
        validation_done = true;
    }

    // Проверка габаритов: объекты в цилиндре радиуса 2.5 м
    // вокруг осей путей (промт п.25, упрощённо)
    if (ImGui::Button("Проверка габаритов (2.5 м)"))
    {
        const auto clearance_issues = Validator::check_clearance(context_);

        validation_issues.insert(validation_issues.end(),
            clearance_issues.begin(), clearance_issues.end());

        validation_done = true;
    }

    if (!validation_done)
    {
        ImGui::TextUnformatted("Click \"Run validation\" to check the route");
        ImGui::End();
        return;
    }

    std::size_t errors = 0;
    std::size_t warnings = 0;

    for (const auto& issue : validation_issues)
    {
        (issue.type == ValidationType::Error ? errors : warnings)++;
    }

    ImGui::Text("Found: %zu error(s), %zu warning(s)",
        errors, warnings);

    ImGui::Spacing();

    if (validation_issues.empty())
    {
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f),
            "No issues found");
        ImGui::End();
        return;
    }

    if (ImGui::BeginTable("validator_table", 2,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg))
    {
        for (std::size_t i = 0; i < validation_issues.size(); ++i)
        {
            const auto& issue = validation_issues[i];

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            ImGui::PushID(static_cast<int>(i));

            if (issue.has_position && ImGui::Button("Go"))
            {
                const auto look_at = context_.camera->get_look_at();

                look_at->eye = issue.position + vsg::dvec3(0.0, 0.0, 50.0);
                look_at->center = look_at->eye +
                    context_.event_handler->get_front();
            }

            ImGui::PopID();

            ImGui::TableNextColumn();

            if (issue.type == ValidationType::Error)
            {
                ImGui::PushStyleColor(ImGuiCol_Text,
                    ImVec4(1.0f, 0.35f, 0.35f, 1.0f));
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text,
                    ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
            }

            ImGui::TextUnformatted(issue.text.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void EditorGui::draw_model_browser_window() const
{
    if (!ImGui::Begin("Model browser", nullptr))
    {
        ImGui::End();
        return;
    }

    if (context_.objects_ref.empty())
    {
        ImGui::TextUnformatted("There is no route yet");
        ImGui::End();
        return;
    }

    // Категория = первая папка пути модели из objects.ref
    std::map<std::string, std::vector<const std::string*>> category_labels;

    for (const auto& [label, ref] : context_.objects_ref)
    {
        std::string category = "(root)";

        const std::filesystem::path model_path(ref.relative_path);

        if (model_path.has_parent_path())
        {
            category = (*model_path.begin()).string();
        }

        category_labels[category].push_back(&label);
    }

    if (model_browser_category >
        static_cast<int>(category_labels.size()))
    {
        model_browser_category = 0;
    }

    // Слева - дерево категорий (0 - «Все»)
    ImGui::TextUnformatted("Categories");

    ImGui::BeginChild("model_categories", ImVec2(220.0f, 0.0f),
        ImGuiChildFlags_Borders);

    if (ImGui::Selectable("All", model_browser_category == 0))
    {
        model_browser_category = 0;
    }

    int category_index = 1;

    for (const auto& [category, labels] : category_labels)
    {
        ImGui::PushID(category.c_str());

        if (ImGui::Selectable(category.c_str(),
            model_browser_category == category_index))
        {
            model_browser_category = category_index;
        }

        ImGui::PopID();

        ++category_index;
    }

    ImGui::EndChild();

    // Справа - список меток с поиском
    ImGui::SameLine();

    ImGui::BeginChild("model_list", ImVec2(0.0f, 0.0f),
        ImGuiChildFlags_Borders);

    ImGui::InputTextWithHint("search", "поиск по метке",
        model_browser_filter, sizeof(model_browser_filter));

    std::string search_lower = model_browser_filter;
    std::transform(search_lower.begin(), search_lower.end(),
        search_lower.begin(), ::tolower);

    const auto draw_label = [&](const std::string& label) -> void
    {
        std::string label_lower = label;
        std::transform(label_lower.begin(), label_lower.end(),
            label_lower.begin(), ::tolower);

        if (model_browser_filter[0] != '\0' &&
            label_lower.find(search_lower) == std::string::npos)
        {
            return;
        }

        ImGui::PushID(label.c_str());

        // Избранное: звёздочка у метки
        const bool favorite = is_favorite(label);

        if (ImGui::SmallButton(favorite ? "*" : "-"))
        {
            toggle_favorite(label);
        }

        ImGui::SetTooltip("%s", favorite ? "Убрать из избранного"
                                         : "Добавить в избранное");

        ImGui::SameLine();

        // Двойной клик по строке - добавление в сцену
        if (ImGui::Selectable(label.c_str(), false,
            ImGuiSelectableFlags_AllowDoubleClick))
        {
            if (ImGui::IsMouseDoubleClicked(0))
            {
                add_object(label);
            }
        }

        ImGui::SameLine();

        if (ImGui::SmallButton("Добавить"))
        {
            add_object(label);
        }

        ImGui::SameLine();

        // Режим установки кликом (TSRE-style): серия объектов по земле
        const bool placing = context_.place_object_mode &&
                             (context_.place_object_label == label);

        if (ImGui::SmallButton(placing ? u8"Ставлю (Esc)" : u8"Кликом"))
        {
            if (placing)
            {
                context_.place_object_mode = false;
                context_.place_object_label.clear();
            }
            else
            {
                context_.place_object_mode = true;
                context_.place_object_label = label;
                context_.place_object_rotation_deg = 0.0;
                context_.status = "Установка: ЛКМ - поставить, [ ] - поворот, Esc - выход";
            }
        }

        ImGui::PopID();
    };

    if (model_browser_category == 0)
    {
        for (const auto& [category, labels] : category_labels)
        {
            for (const std::string* const label : labels)
            {
                draw_label(*label);
            }
        }
    }
    else
    {
        int category_index = 1;

        for (const auto& [category, labels] : category_labels)
        {
            if (category_index == model_browser_category)
            {
                for (const std::string* const label : labels)
                {
                    draw_label(*label);
                }

                break;
            }

            ++category_index;
        }
    }

    ImGui::EndChild();

    ImGui::End();
}

void EditorGui::draw_mass_edit_window() const
{
    if (!ImGui::Begin("Mass edit", nullptr))
    {
        ImGui::End();
        return;
    }

    if (context_.state != EditorState::EDIT_ROUTE)
    {
        ImGui::TextUnformatted("There is no route yet");
        ImGui::End();
        return;
    }

    ImGui::InputText("Фильтр по метке", mass_edit_filter,
        sizeof(mass_edit_filter));
    ImGui::InputText("Новая метка", mass_edit_new_label,
        sizeof(mass_edit_new_label));

    // Новая метка обязана существовать в objects.ref
    const std::string new_label(mass_edit_new_label);

    const bool new_label_valid = !new_label.empty() &&
        context_.objects_ref.find(new_label) != context_.objects_ref.end();

    if (!new_label.empty())
    {
        if (new_label_valid)
        {
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f),
                "Метка найдена в objects.ref");
        }
        else
        {
            ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f),
                "Метки нет в objects.ref");
        }
    }

    // Отфильтрованные объекты (копия списка под мьютексом)
    std::string filter_lower = mass_edit_filter;
    std::transform(filter_lower.begin(), filter_lower.end(),
        filter_lower.begin(), ::tolower);

    RouteObjects filtered_objects;

    {
        std::lock_guard<std::mutex> lock_guard(
            context_.static_objects_mutex);

        for (const auto& object : context_.static_objects)
        {
            std::string label_lower = object->label;
            std::transform(label_lower.begin(), label_lower.end(),
                label_lower.begin(), ::tolower);

            if (mass_edit_filter[0] != '\0' &&
                label_lower.find(filter_lower) == std::string::npos)
            {
                continue;
            }

            filtered_objects.emplace_back(object);
        }
    }

    ImGui::Text("Подходящих объектов: %zu", filtered_objects.size());

    ImGui::BeginDisabled(filtered_objects.empty() || !new_label_valid);

    const std::string replace_button_text = "Заменить метку (" +
        std::to_string(filtered_objects.size()) + ")";

    if (ImGui::Button(replace_button_text.c_str()))
    {
        context_.commands.push(
            new ReplaceLabel(context_, filtered_objects, new_label), true);
    }

    ImGui::EndDisabled();

    ImGui::BeginDisabled(filtered_objects.empty());

    if (ImGui::Button("Сброс масштаба"))
    {
        context_.commands.push(
            new ResetScale(context_, filtered_objects), true);
    }

    const std::string delete_button_text = "Удалить отфильтрованные (" +
        std::to_string(filtered_objects.size()) + ")";

    if (ImGui::Button(delete_button_text.c_str()))
    {
        context_.commands.push(
            new DeleteObjects(context_, filtered_objects), true);

        context_.status = "Удалено объектов: " +
            std::to_string(filtered_objects.size());
    }

    ImGui::EndDisabled();

    ImGui::End();
}

void EditorGui::draw_layers_window() const
{
    if (!ImGui::Begin("Слои", nullptr))
    {
        ImGui::End();
        return;
    }

    if (context_.state != EditorState::EDIT_ROUTE)
    {
        ImGui::TextUnformatted("There is no route yet");
        ImGui::End();
        return;
    }

    // Слои и число объектов в них (копия под мьютексом)
    std::map<std::string, std::size_t> layer_counts;

    {
        std::lock_guard<std::mutex> lock_guard(
            context_.static_objects_mutex);

        for (const auto& object : context_.static_objects)
        {
            const std::string object_layer = object->layer.empty()
                ? "default" : object->layer;

            ++layer_counts[object_layer];
        }
    }

    if (layer_counts.empty())
    {
        ImGui::TextUnformatted("Объектов нет");
        ImGui::End();
        return;
    }

    std::vector<std::string> layer_names;
    layer_names.reserve(layer_counts.size());

    for (const auto& [layer_name, count] : layer_counts)
    {
        layer_names.push_back(layer_name);
    }

    // Назначение слоя выделенным объектам
    ImGui::TextUnformatted("Назначить слой выделенным");

    if (layer_assign_index >= static_cast<int>(layer_names.size()))
    {
        layer_assign_index = 0;
    }

    ImGui::Combo("Слой", &layer_assign_index, layer_name_getter,
        &layer_names, static_cast<int>(layer_names.size()));

    ImGui::InputText("Новый слой", layer_new_name,
        sizeof(layer_new_name));

    if (ImGui::Button("Назначить выделенным"))
    {
        // Имя из поля приоритетнее выбранного в комбо
        const std::string new_layer_name(layer_new_name);

        const std::string target_layer = !new_layer_name.empty()
            ? new_layer_name
            : layer_names.at(static_cast<std::size_t>(
                layer_assign_index));

        if (!target_layer.empty())
        {
            std::size_t count = 0;

            for (const auto& object : context_.selected_objects)
            {
                object->layer = target_layer;
                ++count;
            }

            context_.status = "Слой \"" + target_layer +
                "\" назначен " + std::to_string(count) + " объектам";
        }
    }

    ImGui::Separator();

    // Видимость слоёв: маска узлов объектов слоя
    // (тот же механизм, что у ToggleVisibility)
    ImGui::TextUnformatted("Видимость слоёв");

    for (const auto& [layer_name, count] : layer_counts)
    {
        ImGui::PushID(layer_name.c_str());

        // Слой без записи в карте видимости считается видимым
        bool visible = true;

        const auto visibility_it = layer_visibility_.find(layer_name);

        if (visibility_it != layer_visibility_.cend())
        {
            visible = visibility_it->second;
        }

        if (ImGui::Checkbox("##visibility", &visible))
        {
            layer_visibility_[layer_name] = visible;

            const vsg::Mask mask =
                visible ? vsg::MASK_ALL : vsg::MASK_OFF;

            std::lock_guard<std::mutex> lock_guard(
                context_.static_objects_mutex);

            for (const auto& object : context_.static_objects)
            {
                const std::string object_layer = object->layer.empty()
                    ? "default" : object->layer;

                if (object_layer == layer_name)
                {
                    object->mask = mask;
                }
            }
        }

        ImGui::SameLine();

        ImGui::Text("%s (%zu)", layer_name.c_str(), count);

        ImGui::PopID();
    }

    ImGui::End();
}

void EditorGui::draw_prefabs_window() const
{
    if (!ImGui::Begin("Префабы", nullptr))
    {
        ImGui::End();
        return;
    }

    if (context_.state != EditorState::EDIT_ROUTE)
    {
        ImGui::TextUnformatted("There is no route yet");
        ImGui::End();
        return;
    }

    // Сохранение выделенного набора объектов как префаба
    ImGui::InputText("Имя префаба", prefab_name, sizeof(prefab_name));

    if (ImGui::Button("Сохранить выделенное как префаб"))
    {
        const std::string name(prefab_name);

        if (name.empty())
        {
            context_.status = "Префаб: задайте имя";
        }
        else if (context_.selected_objects.empty())
        {
            context_.status = "Префаб: нет выделенных объектов";
        }
        else
        {
            // Каждая запись - один объект префаба: метка + полная
            // трансформация на момент сохранения
            for (const auto& object : context_.selected_objects)
            {
                PrefabConfig prefab_config;
                prefab_config.name = name;
                prefab_config.object_label = object->label;
                prefab_config.position = object->get_translation();
                prefab_config.rotation_deg = object->get_rotation_deg();
                prefab_config.scale = object->get_scale();

                context_.prefab_configs.push_back(
                    std::move(prefab_config));
            }

            save_track_edit_conf();

            context_.status = "Префаб \"" + name + "\": сохранено " +
                std::to_string(context_.selected_objects.size()) +
                " объектов";

            selected_prefab = name;
        }
    }

    ImGui::Separator();

    // Список префабов: имена без повторов + число объектов
    std::vector<std::string> prefab_names;
    std::map<std::string, std::size_t> prefab_counts;

    for (const PrefabConfig& prefab_config : context_.prefab_configs)
    {
        if (std::find(prefab_names.begin(), prefab_names.end(),
                prefab_config.name) == prefab_names.end())
        {
            prefab_names.push_back(prefab_config.name);
        }

        ++prefab_counts[prefab_config.name];
    }

    if (prefab_names.empty())
    {
        ImGui::TextUnformatted("Префабов нет");
        ImGui::End();
        return;
    }

    for (const std::string& name : prefab_names)
    {
        ImGui::PushID(name.c_str());

        const bool is_selected = selected_prefab == name;

        if (ImGui::Selectable(name.c_str(), is_selected))
        {
            selected_prefab = name;
        }

        ImGui::SameLine();
        ImGui::Text("(%zu)", prefab_counts[name]);

        ImGui::SameLine();

        if (ImGui::SmallButton("Вставить у камеры"))
        {
            paste_prefab(name);
        }

        ImGui::PopID();
    }

    ImGui::End();
}

void EditorGui::paste_prefab(const std::string& name) const
{
    // Копии объектов префаба: записи опознаются по метке и позиции
    // (как у слоёв), объект обязан быть в сцене на момент вставки
    RouteObjects copies;
    vsg::dvec3 first_position = {0.0, 0.0, 0.0};
    bool has_first = false;

    {
        std::lock_guard<std::mutex> lock_guard(
            context_.static_objects_mutex);

        for (const PrefabConfig& prefab_config : context_.prefab_configs)
        {
            if (prefab_config.name != name)
            {
                continue;
            }

            for (const auto& object : context_.static_objects)
            {
                if (object->label == prefab_config.object_label &&
                    vsg::length(object->get_translation() -
                        prefab_config.position) < 0.01)
                {
                    copies.emplace_back(object->copy());

                    if (!has_first)
                    {
                        first_position = prefab_config.position;
                        has_first = true;
                    }

                    break;
                }
            }
        }
    }

    if (copies.empty())
    {
        context_.status = "Префаб \"" + name +
            "\": объекты не найдены в сцене";
        return;
    }

    // Точка перед камерой (как при добавлении объекта)
    const auto look_at = context_.camera->get_look_at();

    const vsg::dvec3 target = look_at->eye +
        context_.event_handler->get_front() * 20.0;

    // Общее смещение: первый объект префаба встаёт в точку перед
    // камерой, остальные сохраняют взаимное расположение
    const vsg::dvec3 offset = target - first_position;

    const std::size_t pasted_count = copies.size();

    context_.commands.push(
        new PastePrefab(context_, std::move(copies), offset, name), true);

    context_.status = "Префаб \"" + name + "\": вставлено " +
        std::to_string(pasted_count) + " объектов";
}

void EditorGui::load_favorites()
{
    const FileSystem& fs = FileSystem::getInstance();

    const QString cfg_path = QString::fromStdString(
        fs.combinePath(fs.getConfigDir(), "editor-settings.xml"));

    CfgReader cfg;

    if (!cfg.load(cfg_path))
    {
        // Файла ещё нет - избранное пустое, это не ошибка
        return;
    }

    QDomNode section = cfg.getFirstSection("Favorites");

    while (!section.isNull())
    {
        QString label;

        if (cfg.getString(section, "Item", label) && !label.isEmpty())
        {
            favorites_.push_back(label.toStdString());
        }

        section = cfg.getNextSection();
    }
}

bool EditorGui::save_favorites() const
{
    const FileSystem& fs = FileSystem::getInstance();

    const std::string path = fs.combinePath(fs.getConfigDir(),
        "editor-settings.xml");

    std::string content;

    std::ifstream input(path, std::ios::binary);

    if (input.is_open())
    {
        content.assign(std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>());

        input.close();
    }
    else
    {
        // Файла ещё нет - создаём минимальный конфиг
        content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                  "<Config>\n</Config>\n";
    }

    // Тело секции Favorites с актуальным списком
    std::string section = "    <Favorites>\n";

    for (const std::string& label : favorites_)
    {
        section += "        <Item>" + label + "</Item>\n";
    }

    section += "    </Favorites>";

    std::string result;

    const std::size_t begin = content.find("<Favorites>");

    if (begin != std::string::npos)
    {
        // Заменяем существующую секцию целиком (как KeyBindings::save)
        const std::size_t end = content.find("</Favorites>", begin);

        if (end == std::string::npos)
        {
            return false;
        }

        const std::size_t begin_tag = section.find("<Favorites>");

        result = content.substr(0, begin) +
            section.substr(begin_tag) +
            content.substr(end + std::string("</Favorites>").size());
    }
    else
    {
        // Секции ещё нет - добавляем в конец контейнера Config
        const std::size_t config_end = content.find("</Config>");

        if (config_end == std::string::npos)
        {
            return false;
        }

        result = content.substr(0, config_end) + section + "\n" +
            content.substr(config_end);
    }

    std::ofstream output(path, std::ios::binary | std::ios::trunc);

    if (!output.is_open())
    {
        return false;
    }

    output << result;

    return output.good();
}

bool EditorGui::is_favorite(const std::string& label) const
{
    return std::find(favorites_.cbegin(), favorites_.cend(), label) !=
        favorites_.cend();
}

void EditorGui::toggle_favorite(const std::string& label) const
{
    const auto favorite_it = std::find(favorites_.begin(),
        favorites_.end(), label);

    if (favorite_it != favorites_.end())
    {
        favorites_.erase(favorite_it);
    }
    else
    {
        favorites_.push_back(label);
    }

    if (!save_favorites())
    {
        context_.status =
            "Не удалось сохранить избранное в editor-settings.xml";
    }
}

void EditorGui::draw_selection_rect() const
{
    if (!context_.selection_rect_active)
    {
        return;
    }

    ImDrawList* draw_list = ImGui::GetForegroundDrawList();

    const ImVec2 a(static_cast<float>(context_.selection_rect_start.x),
                   static_cast<float>(context_.selection_rect_start.y));
    const ImVec2 b(static_cast<float>(context_.selection_rect_curr.x),
                   static_cast<float>(context_.selection_rect_curr.y));

    draw_list->AddRectFilled(a, b,
        IM_COL32(80, 160, 255, 40), 0.0f, 0);
    draw_list->AddRect(a, b,
        IM_COL32(80, 160, 255, 200), 0.0f, 0, 1.5f);
}

void EditorGui::add_object(const std::string& label) const
{
    const auto ref_it = context_.objects_ref.find(label);

    if (ref_it == context_.objects_ref.end() || !ref_it->second.paged_lod)
    {
        return;
    }

    const auto look_at = context_.camera->get_look_at();

    const auto object = RouteObject::create(context_, ref_it->second.paged_lod,
        label, look_at->eye + context_.event_handler->get_front() * 20.0);

    context_.commands.push(new AddObject(context_, object), true);
}

ObjectDragState& EditorGui::get_drag_state(const RouteObject* object) const
{
    // Убираем состояния объектов, которых больше нет в выделении,
    // чтобы карта не росла бесконечно
    for (auto it = drag_states_.begin(); it != drag_states_.end(); )
    {
        const bool is_selected = std::any_of(
            context_.selected_objects.cbegin(),
            context_.selected_objects.cend(),
            [&it](const vsg::ref_ptr<RouteObject>& selected) -> bool {
                return selected.get() == it->first;
            });

        if (!is_selected)
        {
            it = drag_states_.erase(it);
        }
        else
        {
            ++it;
        }
    }

    return drag_states_[object];
}

void EditorGui::save_objects_matrixes() const
{
    for (const auto& object : context_.selected_objects)
    {
        object->save_matrix();
    }
}

void EditorGui::handle_translation_drag(
    std::size_t index,
    vsg::ref_ptr<RouteObject> object,
    ObjectDragState& drag_state
) const
{
    const std::string label = "translation##" + std::to_string(index);

    vsg::dvec3 translation = object->get_translation();
    if (drag_double3(label.c_str(), translation.data()))
    {
        if (!drag_state.dragging)
        {
            drag_state.total_translation = {0.0, 0.0, 0.0};
            save_objects_matrixes();
            drag_state.dragging = true;
        }
        drag_state.total_translation += translation - object->get_translation();
        object->set_translation(translation);
    }

    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        context_.commands.push(new TranslateObjects(context_, {object},
            drag_state.total_translation), false);
        drag_state.dragging = false;
    }
}

void EditorGui::handle_rotation_drag(
    std::size_t index,
    vsg::ref_ptr<RouteObject> object,
    ObjectDragState& drag_state
) const
{
    const std::string label = "rotation##" + std::to_string(index);

    constexpr double min_rot_deg = -360.0;
    constexpr double max_rot_deg = 360.0;
    vsg::dvec3 rotation_deg = object->get_rotation_deg();
    if (drag_double3(label.c_str(), rotation_deg.data(), 1.0f,
        &min_rot_deg, &max_rot_deg, ImGuiSliderFlags_WrapAround))
    {
        if (!drag_state.dragging)
        {
            drag_state.total_rotation_deg = {0.0, 0.0, 0.0};
            save_objects_matrixes();
            drag_state.dragging = true;
        }
        drag_state.total_rotation_deg += rotation_deg - object->get_rotation_deg();
        object->set_rotation_deg(rotation_deg);
    }

    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        vsg::dvec3 axis = {0.0, 0.0, 0.0};
        double radians;

        if (std::abs(drag_state.total_rotation_deg.x) >= 1.0e-6)
        {
            axis.x = 1.0;
            radians = vsg::radians(drag_state.total_rotation_deg.x);
        }
        else if (std::abs(drag_state.total_rotation_deg.y) >= 1.0e-6)
        {
            axis.y = 1.0;
            radians = vsg::radians(drag_state.total_rotation_deg.y);
        }
        else
        {
            axis.z = 1.0;
            radians = vsg::radians(drag_state.total_rotation_deg.z);
        }

        // Вращаем вокруг центра bbox самого объекта
        const vsg::dbox& bounds = object->get_bounds();
        const vsg::dvec3 pivot = 0.5 * (bounds.min + bounds.max);

        context_.commands.push(new RotateObjects(context_, {object},
            pivot, axis, radians), false);
        drag_state.dragging = false;
    }
}

void EditorGui::handle_scale_drag(
    std::size_t index,
    vsg::ref_ptr<RouteObject> object,
    ObjectDragState& drag_state
) const
{
    const std::string label = "scale##" + std::to_string(index);

    const vsg::dvec3& prev_scale = object->get_scale();
    vsg::dvec3 scale = object->get_scale();
    if (drag_double3(label.c_str(), scale.data(), 0.01f))
    {
        if (vsg::length(scale) > 1.0e-6)
        {
            if (!drag_state.dragging)
            {
                drag_state.total_scale = {1.0, 1.0, 1.0};
                save_objects_matrixes();
                drag_state.dragging = true;
            }
            drag_state.total_scale *= {scale.x / prev_scale.x,
                scale.y / prev_scale.y, scale.z / prev_scale.z};
            object->set_scale(scale);
        }
    }

    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        // Масштабируем относительно центра bbox самого объекта
        const vsg::dbox& bounds = object->get_bounds();
        const vsg::dvec3 pivot = 0.5 * (bounds.min + bounds.max);

        context_.commands.push(new ScaleObjects(context_, {object},
            pivot, drag_state.total_scale), false);
        drag_state.dragging = false;
    }
}

bool EditorGui::create_route_structure(const std::string& parent_dir,
                                       const std::string& name) const
{
    namespace fs = std::filesystem;

    std::error_code error_code;

    const fs::path route_dir = fs::path(parent_dir) / name;

    if (fs::exists(route_dir, error_code))
    {
        context_.status = "Route directory already exists";
        return false;
    }

    // Структура каталогов нового маршрута
    const fs::path subdirs[] = {
        route_dir / "models",
        route_dir / "textures",
        route_dir / "topology" / "map",
        route_dir / "topology" / "trajectories"
    };

    for (const fs::path& dir : subdirs)
    {
        if (!fs::create_directories(dir, error_code))
        {
            Journal::instance()->error(QString("Failed to create directory %1")
                .arg(dir.string().c_str()));

            return false;
        }
    }

    // Пустой objects.ref
    {
        const fs::path path = route_dir / "objects.ref";
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            return false;
        }
    }

    // Пустые станции и путевые точки
    {
        const fs::path paths[] = {
            route_dir / "topology" / "stations.conf",
            route_dir / "topology" / "waypoints.conf"
        };

        for (const fs::path& path : paths)
        {
            std::ofstream file(path, std::ios::binary);
            if (!file.is_open())
            {
                return false;
            }
        }
    }

    // Пустая карта объектов
    {
        const fs::path path = route_dir / "topology" / "map" / "route1.map";
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            return false;
        }
    }

    // Заготовка топологии
    {
        const fs::path path = route_dir / "topology" / "topology.xml";
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            return false;
        }
        file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        file << "<Config>\n";
        file << "</Config>\n";
    }

    // Каталог моделей сигналов (пустой, задаётся в models-config.xml)
    {
        const fs::path path = route_dir / "topology" / "models-config.xml";
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            return false;
        }
        file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        file << "<Config>\n";
        file << "    <Models>\n";
        file << "        <SignalModelsDir></SignalModelsDir>\n";
        file << "    </Models>\n";
        file << "</Config>\n";
    }

    // Открываем созданный маршрут
    context_.route_dir = route_dir.string();
    context_.state = EditorState::LOAD_ROUTE;

    Journal::instance()->info(QString("Created route structure %1")
        .arg(route_dir.string().c_str()));

    return true;
}
