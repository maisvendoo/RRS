#include "editor/EditorGui.h"

#include "editor/Camera.h"
#include "editor/EditorContext.h"
#include "editor/EventHandler.h"
#include "editor/Route.h"
#include "editor/RouteObject.h"
#include "editor/commands/AddObject.h"
#include "editor/commands/CommandList.h"
#include "editor/commands/RotateObjects.h"
#include "editor/commands/ScaleObjects.h"
#include "editor/commands/TranslateObjects.h"
#include "editor/settings/CameraSettings.h"
#include "editor/settings/GuiSettings.h"
#include "editor/states/EditorState.h"

#include <Journal.h>
#include <filesystem.h>

#include <ImGuiFileDialog.h>
#include <vsg/app/Viewer.h>
#include <vsg/maths/common.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsgImGui/imgui.h>

#include <algorithm>
#include <cctype>
#include <cfloat>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

static bool drag_double3(const char* label, double* data, float speed = 1.0f,
    const double* min = nullptr, const double* max = nullptr,
    ImGuiSliderFlags flags = 0)
{
    return ImGui::DragScalarN(label, ImGuiDataType_Double, data, 3,
        speed, min, max, "%.3f", flags);
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

    if (gui_settings.show_selected_objects_properties)
    {
        draw_selected_objects_window();
    }

    if (gui_settings.show_commands)
    {
        draw_commands_window();
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
        ImGui::MenuItem("Selected objects", nullptr,
            &context_.gui_settings.show_selected_objects_properties);
        ImGui::MenuItem("Commands", nullptr,
            &context_.gui_settings.show_commands);

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

        if (!std::filesystem::exists(route_dir + "/models") ||
            !std::filesystem::exists(route_dir + "/textures") ||
            !std::filesystem::exists(route_dir + "/topology") ||
            !std::filesystem::exists(route_dir + "/objects.ref"))
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
