#include "editor/MeasureTool.h"

#include "editor/EditorContext.h"
#include "editor/IntersectionHandler.h"
#include "editor/MouseButton.h"
#include "editor/ScreenProjector.h"
#include "editor/states/EditorState.h"

#include <vsgImGui/imgui.h>

#include <vsg/nodes/Group.h>
#include <vsg/ui/PointerEvent.h>

#include <cmath>
#include <cstdio>
#include <string>

MeasureTool::MeasureTool(EditorContext& context)
    : context_(context)
{
}

void MeasureTool::set_active(bool active)
{
    active_ = active;
}

bool MeasureTool::is_active() const
{
    return active_;
}

void MeasureTool::handle_press(const vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled ||
        buttonPress.button != editor2::MOUSE_BUTTON_LEFT)
    {
        return;
    }

    // Клик по окну ImGui не должен ставить точку
    if (ImGui::GetCurrentContext() != nullptr &&
        ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    if (context_.state != EditorState::EDIT_ROUTE)
    {
        return;
    }

    // Луч камеры через позицию курсора (создаётся IntersectionHandler)
    const LSIntersectorRefPtr intersector =
        context_.intersection_handler->get_lmb_intersector();

    if (!intersector)
    {
        return;
    }

    context_.scenegraph->accept(*intersector);

    const LSIntersectionRefPtr intersection =
        IntersectionHandler::get_closest_intersection(intersector);

    if (intersection)
    {
        points_.push_back(intersection->worldIntersection);
    }

    intersector->intersections.clear();
}

void MeasureTool::clear()
{
    points_.clear();
}

void MeasureTool::draw_overlay()
{
    if (!active_)
    {
        return;
    }

    // Точки и полилиния: экранные проекции пересчитываются каждый кадр
    ImDrawList* const draw_list = ImGui::GetBackgroundDrawList();

    bool has_prev_screen = false;
    ImVec2 prev_screen = {0.0f, 0.0f};

    for (const vsg::dvec3& point : points_)
    {
        vsg::dvec2 screen = {0.0, 0.0};

        if (!editor2::project_world_to_screen(context_, point, screen))
        {
            has_prev_screen = false;
            continue;
        }

        const ImVec2 curr_screen(static_cast<float>(screen.x),
            static_cast<float>(screen.y));

        if (has_prev_screen)
        {
            draw_list->AddLine(prev_screen, curr_screen,
                IM_COL32(0, 255, 255, 220), 2.0f);
        }

        draw_list->AddCircleFilled(curr_screen, 5.0f,
            IM_COL32(255, 210, 0, 255), 12);

        prev_screen = curr_screen;
        has_prev_screen = true;
    }

    // Окно со длинами сегментов (правый нижний угол)
    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(
        ImVec2(io.DisplaySize.x - 16.0f, io.DisplaySize.y - 16.0f),
        ImGuiCond_Always, ImVec2(1.0f, 1.0f));

    if (!ImGui::Begin("Измерение", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoFocusOnAppearing))
    {
        ImGui::End();
        return;
    }

    if (points_.empty())
    {
        ImGui::TextUnformatted(
            "Кликните по сцене, чтобы поставить точку (M - выход)");
        ImGui::End();
        return;
    }

    if (ImGui::BeginTable("measure_table", 3,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Точки");
        ImGui::TableSetupColumn("Длина, м");
        ImGui::TableSetupColumn("Дельта Z, м");
        ImGui::TableHeadersRow();

        for (std::size_t i = 1; i < points_.size(); ++i)
        {
            const vsg::dvec3& begin = points_[i - 1];
            const vsg::dvec3& end = points_[i];

            const double length = vsg::length(end - begin);
            const double delta_z = end.z - begin.z;

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            char label[64];
            std::snprintf(label, sizeof(label), "%zu - %zu", i, i + 1);
            ImGui::TextUnformatted(label);

            ImGui::TableNextColumn();
            ImGui::Text("%10.3f", length);

            ImGui::TableNextColumn();
            ImGui::Text("%10.3f", delta_z);
        }

        ImGui::EndTable();

        // Суммарная длина всей полилинии
        if (points_.size() >= 2)
        {
            double total_length = 0.0;

            for (std::size_t i = 1; i < points_.size(); ++i)
            {
                total_length += vsg::length(points_[i] - points_[i - 1]);
            }

            ImGui::Text("Всего: %.3f м", total_length);
        }
    }

    if (ImGui::Button("Сброс"))
    {
        clear();
    }

    ImGui::SameLine();
    ImGui::TextUnformatted("M - выход");

    ImGui::End();
}
