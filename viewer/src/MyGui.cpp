#include "MyGui.h"
#include "CabMouseHandler.h"

#include "Logger.h"
#include "filesystem.h"
#include "datetime.h"

#include "NewSkybox.h"
#include "RouteViewer.h"
// #include "Skybox.h"
#include "Sun.h"
#include "UpdateStatisticsHandler.h"
#include "UpdateControlToServerHandler.h"
#include "VehiclesHandler.h"
#include <tcp-client.h>

#include <vsg/io/Options.h>
#include <vsg/maths/common.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/transform.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/app/Viewer.h>
#include <vsgImGui/imgui.h>

#include <algorithm>

#define IMGUI_ENABLE_STD_STRING

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MyGui::MyGui(vsg::ref_ptr<GUIParams> in_params, [[maybe_unused]] vsg::ref_ptr<vsg::Options> options)
    : params(in_params)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    FileSystem &fs = FileSystem::getInstance();

    std::string font_path = fs.getFontsDir() + fs.separator() + "JetBrainsMono-Regular.ttf";

    io.Fonts->AddFontFromFileTTF(font_path.c_str(),
                                 font_size,
                                 NULL,
                                 io.Fonts->GetGlyphRangesCyrillic());

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::compile([[maybe_unused]] vsg::Context& context)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::record([[maybe_unused]] vsg::CommandBuffer& cb) const
{
    bool is_modified_key = ImGui::IsKeyPressed(ImGuiKey_LeftShift) ||
                           ImGui::IsKeyPressed(ImGuiKey_RightShift) ||
                           ImGui::IsKeyPressed(ImGuiKey_LeftCtrl) ||
                           ImGui::IsKeyPressed(ImGuiKey_RightCtrl) ||
                           ImGui::IsKeyPressed(ImGuiKey_LeftAlt) ||
                           ImGui::IsKeyPressed(ImGuiKey_RightAlt);

    // Статус вьювера
    if (!params->status.isEmpty())
    {
        showStatus();
    }

    // Подтверждение выхода по Esc
    if (ImGui::IsKeyPressed(ImGuiKey_Escape) && !params->prev_Esc)
    {
        params->is_show_quit_dialog = !params->is_show_quit_dialog;
    }
    params->prev_Esc = ImGui::IsKeyPressed(ImGuiKey_Escape);

    if (params->is_show_quit_dialog)
    {
        showQuitDialog();
    }

    // Отображение статистики
    if (ImGui::IsKeyPressed(ImGuiKey_F11) && !params->prev_F11 && !is_modified_key)
    {
        params->is_show_statistics = !params->is_show_statistics;
    }
    params->prev_F11 = ImGui::IsKeyPressed(ImGuiKey_F11);

    if (params->is_show_statistics)
    {
        showStatistics();
    }

    // Отображение настроек
    if (ImGui::IsKeyPressed(ImGuiKey_F10) && !params->prev_F10 && !is_modified_key)
    {
        params->is_show_settings = !params->is_show_settings;
    }
    params->prev_F10 = ImGui::IsKeyPressed(ImGuiKey_F10);

    if (params->is_show_settings)
    {
        showSettings();
    }

    // Отображение диалога переименования поезда
    if (ImGui::IsKeyPressed(ImGuiKey_F8) && !params->prev_F8 && !is_modified_key)
    {
        params->is_show_trane_rename_dialog = !params->is_show_trane_rename_dialog;
    }
    params->prev_F8 = ImGui::IsKeyPressed(ImGuiKey_F8);

    if (params->is_show_trane_rename_dialog)
    {
        showTrainRenameDialog();
    }

    // Отображение HUD
    if (ImGui::IsKeyPressed(ImGuiKey_F7) && !params->prev_F7 && !is_modified_key)
    {
        params->is_show_HUD = !params->is_show_HUD;
    }
    params->prev_F7 = ImGui::IsKeyPressed(ImGuiKey_F7);

    if (params->is_show_HUD)
    {
        showHUD();
    }

    // Подсказка органа кабины под курсором (Alt удерживается)
    showCabTooltip();

    // Диагностика составов (ТЗ "Промт статистики вагонов"):
    // F3 - вкл/выкл окна, F4 - свёрнутый/полный режим.
    // Камеры внешняя/свободная перенесены на Shift+F3/Shift+F4
    if (ImGui::IsKeyPressed(ImGuiKey_F3) && !params->prev_F3 && !is_modified_key)
    {
        params->is_show_diagnostics = !params->is_show_diagnostics;
    }
    params->prev_F3 = ImGui::IsKeyPressed(ImGuiKey_F3);

    if (ImGui::IsKeyPressed(ImGuiKey_F4) && !params->prev_F4 && !is_modified_key)
    {
        params->diagnostics_full_mode = !params->diagnostics_full_mode;
    }
    params->prev_F4 = ImGui::IsKeyPressed(ImGuiKey_F4);

    if (params->is_show_diagnostics)
    {
        showDiagnostics();
    }

    if (params->vehicles_handler)
    {
        params->speed_factor = params->vehicles_handler->getSpeedFactor();
        params->sim_time = params->vehicles_handler->getDateTime();
        if (params->sim_time && params->use_server_time)
        {
            params->year = params->sim_time->date.year();
            params->month = params->sim_time->date.month();
            params->day = params->sim_time->date.day();
            params->hour = params->sim_time->time.hour();
            params->minute = params->sim_time->time.minute();
            params->sec = params->sim_time->time.sec();
            params->msec = params->sim_time->time.msec();
        }
        else
        {
            check_date_time();
        }

        // Отображение дебаг-строки подвижного состава
        if (ImGui::IsKeyPressed(ImGuiKey_F9) && !params->prev_F9)
        {
            params->is_show_debug_msg = !params->is_show_debug_msg;
            params->controls_handler->setNeedDebugMsg(params->is_show_debug_msg);
        }
        params->prev_F9 = ImGui::IsKeyPressed(ImGuiKey_F9);

        // Строка нажмите Enter для управления
        if (params->vehicles_handler)
        {
            params->is_no_controlled =
                            (params->vehicles_handler->getCurrentVehicleIndex() !=
                            params->vehicles_handler->getControlledVehicleIndex());

            VehicleExterior* cur_vehicle = params->vehicles_handler->getCurrentVehicle();
            params->is_no_cabine_control = ((cur_vehicle != nullptr) &&
                                            (cur_vehicle->controlled_cabine_idx != cur_vehicle->current_cabine_idx));
        }
    }
    else
    {
        params->prev_F9 = false;
        params->is_show_debug_msg = false;
        params->is_no_controlled = false;

        check_date_time();
    }

    simulator_time_t datetime({static_cast<int16_t>(params->year), static_cast<uint8_t>(params->month), static_cast<uint8_t>(params->day)},
                              {static_cast<uint8_t>(params->hour), static_cast<uint8_t>(params->minute), static_cast<uint8_t>(params->sec), static_cast<uint8_t>(params->msec)});

    if (params->sun)
    {
        params->sun->update(datetime, 3.0, params->latitude, params->longitude);
    }

    if (params->new_skybox)
    {
        params->new_skybox->set_date_time(datetime);
        if (params->sun)
        {
            params->new_skybox->set_sun_direction(params->sun->azimuth_deg,
                                                  params->sun->altitude_deg);
        }
    }

    if (params->is_show_debug_msg)
    {
        showDebugMsg();
    }

    // Предупреждение кассеты регистрации (ТЗ "Кассеты"): всплывает
    // по центру экрана на ~5 с при вставке/извлечении (Ctrl+R)
    showCassetteNotice();

    // Автоподсказка посадки на сиденье (E)
    drawWalkHint();

    if (params->is_no_controlled)
    {
        showNoControlled();
    }

    if (params->is_no_cabine_control)
    {
        showNoCabineControl();
    }

    if (params->speed_factor == 0)
    {
        showPauseState();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::showStatus() const
{
    int w = 400;
    int h = 150;

    ImGui::SetNextWindowSize(ImVec2(w, h));

    ImGuiIO &io = ImGui::GetIO();

    ImVec2 content_size = io.DisplaySize;

    ImGui::SetNextWindowPos(ImVec2( (content_size.x - w) / 2, (content_size.y - h) / 2));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoInputs;

    bool open_ptr = true;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
    ImGui::Begin(u8"Загрузка... Пожалуйста, подождите...", &open_ptr, window_flags);
    ImGui::PopStyleColor();
    ImGui::Text(u8"%s", params->status.toStdString().c_str());
    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::showQuitDialog() const
{
    int w = 400;
    int h = 150;

    int cx = w / 2;
    int cy = h / 2;

    ImGui::SetNextWindowSize(ImVec2(w, h));

    ImGuiIO &io = ImGui::GetIO();

    ImVec2 content_size = io.DisplaySize;

    ImGui::SetNextWindowPos(ImVec2( (content_size.x - w) / 2, (content_size.y - h) / 2));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;

    bool open_ptr = true;

    ImGui::Begin(u8"Вы действительно хотите выйти?", &open_ptr, window_flags);

    int bw = w / 4;
    int bh = h / 4;

    ImGui::SetCursorPos(ImVec2(static_cast<int>(cx - 3 * bw / 2), static_cast<int>(cy - bh / 2)));
    if (ImGui::Button(u8"Да", ImVec2(bw, bh)))
    {
        ImGui::SetCursorPos(ImVec2(cx, cy));
        vsg::ref_ptr<vsg::Viewer> viewer = params->viewer;
        if (viewer)
            viewer->close();
    }

    ImGui::SetCursorPos(ImVec2(static_cast<int>(cx + bw / 2), static_cast<int>(cy - bh / 2)));
    if (ImGui::Button(u8"Нет", ImVec2(bw, bh)))
    {
        ImGui::SetCursorPos(ImVec2(cx, cy));
        params->is_show_quit_dialog = false;
    }

    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::showStatistics() const
{
    QString text = QString("Device: %1 ").arg(params->physicalDeviceName);
    text += QString("FPS:%1 (lowest:%2)")
                    .arg(params->statistics_handler->getAverageFPS(), 6, 'f', 1)
                    .arg(params->statistics_handler->getLowestFPS(), 6, 'f', 1);

    ImVec2 text_size = ImGui::CalcTextSize(text.toStdString().c_str());

    ImGuiIO &io = ImGui::GetIO();
    ImVec2 content_size = io.DisplaySize;

    ImGui::SetNextWindowPos(ImVec2(content_size.x - text_size.x - 20, 0));
    ImGui::SetNextWindowSize(ImVec2(text_size.x + 20, text_size.y + 20));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoInputs;

    bool open_ptr = true;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
    ImGui::Begin(u8"Статистика", &open_ptr, window_flags);
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::Text(u8"%s", text.toStdString().c_str());
    ImGui::PopStyleColor();
    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::showSettings() const
{
    ImGui::Begin("Light settings");

    if (params->sim_time)
    {
        ImGui::RadioButton("Время сервера: ", &(params->use_server_time), 1);
        ImGui::SameLine();
        std::string text_server_datetime = params->sim_time->getString(false).toStdString();
        ImGui::Text(u8"%s", text_server_datetime.c_str());
    }
    else
    {
        ImGui::RadioButton("Время сервера: недоступно", false);
    }

    ImGui::RadioButton("Задать время вручную:", &(params->use_server_time), 0);

    ImGuiInputTextFlags flags = params->use_server_time ? ImGuiInputTextFlags_ReadOnly : 0;
    ImGui::PushItemWidth((ImGui::CalcItemWidth() - 2 * ImGui::GetStyle().ItemSpacing.x) / 3);
    constexpr int16_t one = 1;

    ImGui::InputScalar("year", ImGuiDataType_S16, &params->year, &one, NULL, NULL, flags);
    ImGui::SameLine();
    ImGui::InputScalar("month ", ImGuiDataType_S16, &params->month, &one, NULL, NULL, flags);
    ImGui::SameLine();
    ImGui::InputScalar("day", ImGuiDataType_S16, &params->day, &one, NULL, NULL, flags);

    ImGui::InputScalar("hour", ImGuiDataType_S16, &params->hour, &one, NULL, NULL, flags);
    ImGui::SameLine();
    ImGui::InputScalar("minute", ImGuiDataType_S16, &params->minute, &one, NULL, NULL, flags);
    ImGui::SameLine();
    ImGui::InputScalar("sec", ImGuiDataType_S16, &params->sec, &one, NULL, NULL, flags);

    ImGui::PopItemWidth();

    static int day_seconds;
    day_seconds = params->hour * 3600 + params->minute * 60 + params->sec;
    if (ImGui::SliderInt("Day seconds", &day_seconds, 0, 86399))
    {
        params->hour = day_seconds / 3600;
        params->minute = (day_seconds - params->hour * 3600) / 60;
        params->sec = day_seconds % 60;
    }

    if (ImGui::CollapsingHeader("Sun parameters"))
    {
        ImGui::ColorEdit3("Ambient color", params->sun->ambient->color.data());
        ImGui::Checkbox("Set ambient intensity manually", &(params->sun->use_gui_ambient_intensity));
        if (params->sun->use_gui_ambient_intensity)
        {
            ImGui::SliderFloat("Ambient intensity", &(params->sun->ambient->intensity), 0.0f, 1.0f, "%.3f");
        }
        else
        {
            ImGui::Text("Ambient intensity: %.3f", params->sun->ambient->intensity);
        }

        ImGui::ColorEdit3("color", params->sun->sun->color.data());
        ImGui::Checkbox("Set sun intensity manually", &(params->sun->use_gui_sun_intensity));
        if (params->sun->use_gui_sun_intensity)
        {
            // Диапазон до 15: HDR-пресеты High/Ultra задают 10-12
            // (ACES-тонмаппинг сжимает значения в LDR)
            ImGui::SliderFloat("Sun intensity", &(params->sun->sun->intensity), 0.0f, 15.0f, "%.3f");
        }
        else
        {
            ImGui::Text("Sun intensity: %.3f", params->sun->sun->intensity);
        }

        ImGui::Checkbox("Set sun direction manually", &(params->sun->use_gui_sun_direction));
        if (params->sun->use_gui_sun_direction)
        {
            ImGui::SliderFloat("Sun azimuth", &(params->sun->azimuth_deg), 0.0f, 360.0f, "%.3f");
            ImGui::SliderFloat("Sun altitude", &(params->sun->altitude_deg), -90.0f, 90.0f, "%.3f");
        }
        else
        {
            ImGui::Text("Sun azimuth: %.3f", params->sun->azimuth_deg);
            ImGui::Text("Sun altitude: %.3f", params->sun->altitude_deg);
        }
    }

    showGraphicsSettings();

    ImGui::End();
}

//------------------------------------------------------------------------------
// Выбор пресета графики (ТЗ "Графика"): применяется на лету всё, что
// применимо без пересоздания окна; остальное — после перезапуска
//------------------------------------------------------------------------------
void MyGui::showGraphicsSettings() const
{
    if (!ImGui::CollapsingHeader(u8"Настройки графики"))
    {
        return;
    }

    // Индексы совпадают со значениями gfx::Preset
    constexpr const char* preset_names[] =
    {
        "Legacy",
        "Low",
        "High",
        "Ultra",
        "Extreme",
        "Custom"
    };

    int current_preset = params->graphics_preset_index;

    if (ImGui::Combo(u8"Пресет графики", &current_preset,
                     preset_names, IM_ARRAYSIZE(preset_names)))
    {
        // Диапазон придерживаем на всякий случай
        current_preset = std::clamp(current_preset, 0, 5);
        params->graphics_preset_index = current_preset;

        if (params->route_viewer)
        {
            params->route_viewer->setGraphicsPreset(
                static_cast<gfx::Preset>(current_preset), true);

            // Что не применилось без пересоздания окна — сообщаем
            params->graphics_needs_restart =
                params->route_viewer->isGraphicsRestartRequired();
        }
    }

    // Статусы тиров нового качества (ТЗ "Графика"): PBR и ACES-тонмаппинг
    // определяются пресетом и «запекаются» в шейдер-сеты при старте —
    // в GUI только чтение
    ImGui::Text(u8"PBR-материалы: %s", params->graphics_use_pbr ? u8"вкл" : u8"выкл");
    ImGui::Text(u8"ACES-тонмаппинг: %s", params->graphics_use_aces_tonemap ? u8"вкл" : u8"выкл");

    // SSAO: флаг доступен только в Ultra. Сам пасс пост-обработки SSAO
    // в VSG 1.1.x отсутствует и находится в разработке (см.
    // graphics-settings.h), на картинку пока не влияет
    const bool is_ultra =
            (params->graphics_preset_index == static_cast<int>(gfx::Preset::Ultra));

    bool ssao_enabled = params->graphics_use_ssao;

    if (is_ultra)
    {
        if (ImGui::Checkbox(u8"SSAO (в разработке)", &ssao_enabled))
        {
            if (params->route_viewer)
            {
                params->route_viewer->setGraphicsSsaoEnabled(ssao_enabled);
            }
        }
    }
    else
    {
        ImGui::BeginDisabled(true);
        ImGui::Checkbox(u8"SSAO", &ssao_enabled);
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::TextDisabled(u8"(только Ultra, в разработке)");
    }

    // Пост-процесс пресета Extreme (ТЗ "Графика"): Bloom/SSAO/SSR/Туман —
    // реальные проходы цепочки, активны только на Extreme (образец
    // блокировки — BeginDisabled, как у SSAO-чекбокса выше). Изменения
    // применяются после перезапуска (цепочка собирается при старте)
    const bool is_extreme =
            (params->graphics_preset_index == static_cast<int>(gfx::Preset::Extreme));

    bool bloom_enabled = params->graphics_use_bloom;
    bool ssao_pass_enabled = params->graphics_use_ssao_pass;
    bool fog_enabled = params->graphics_use_volumetric_fog;
    bool ssr_enabled = params->graphics_use_ssr;
    float postprocess_scale = params->graphics_postprocess_scale;

    if (is_extreme)
    {
        if (ImGui::Checkbox(u8"Bloom", &bloom_enabled) && params->route_viewer)
        {
            params->route_viewer->setGraphicsPostprocessFlag("use_bloom",
                                                             bloom_enabled);
        }

        if (ImGui::Checkbox(u8"SSAO (пост-процесс)", &ssao_pass_enabled) && params->route_viewer)
        {
            params->route_viewer->setGraphicsPostprocessFlag("use_ssao_pass",
                                                             ssao_pass_enabled);
        }

        if (ImGui::Checkbox(u8"SSR (отражения)", &ssr_enabled) && params->route_viewer)
        {
            params->route_viewer->setGraphicsPostprocessFlag("use_ssr",
                                                             ssr_enabled);
        }

        if (ImGui::Checkbox(u8"Туман", &fog_enabled) && params->route_viewer)
        {
            params->route_viewer->setGraphicsPostprocessFlag("use_volumetric_fog",
                                                             fog_enabled);
        }

        if (ImGui::SliderFloat(u8"Масштаб пост-процесса", &postprocess_scale,
                               0.5f, 1.0f, "%.2f") && params->route_viewer)
        {
            params->route_viewer->setGraphicsPostprocessScale(postprocess_scale);
        }
    }
    else
    {
        ImGui::BeginDisabled(true);
        ImGui::Checkbox(u8"Bloom", &bloom_enabled);
        ImGui::Checkbox(u8"SSAO (пост-процесс)", &ssao_pass_enabled);
        ImGui::Checkbox(u8"SSR (отражения)", &ssr_enabled);
        ImGui::Checkbox(u8"Туман", &fog_enabled);
        ImGui::SliderFloat(u8"Масштаб пост-процесса", &postprocess_scale,
                           0.5f, 1.0f, "%.2f");
        ImGui::EndDisabled();
        ImGui::TextDisabled(u8"(только Extreme)");
    }

    if (params->graphics_needs_restart)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.3f, 1.0f));
        ImGui::TextWrapped(u8"Сглаживание и настройки теней применятся после перезапуска");
        ImGui::PopStyleColor();
    }

    // PBR/ACES запекаются в пайплайн при старте: если смена пресета
    // затронула HDR-тир, перезапуск обязателен (текст — по образцу
    // сообщения про MSAA/тени выше)
    if (params->graphics_needs_restart &&
        ((params->graphics_preset_index == static_cast<int>(gfx::Preset::High)) ||
         (params->graphics_preset_index == static_cast<int>(gfx::Preset::Ultra)) ||
         (params->graphics_preset_index == static_cast<int>(gfx::Preset::Extreme)) ||
         params->graphics_use_pbr || params->graphics_use_aces_tonemap))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.3f, 1.0f));
        ImGui::TextWrapped(u8"PBR и ACES-тонмаппинг применятся после перезапуска");
        ImGui::PopStyleColor();
    }

    // Пост-процесс собирается в командный граф при старте
    if (params->graphics_needs_restart && params->graphics_use_postprocess)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.3f, 1.0f));
        ImGui::TextWrapped(u8"Пост-процесс применится после перезапуска");
        ImGui::PopStyleColor();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::showDebugMsg() const
{
    QString debugMsg = params->vehicles_handler->getDebugMessage();
    QStringList lines = debugMsg.split('\n');
    float h = font_size * (lines.count() + 1);

    ImGuiIO &io = ImGui::GetIO();
    ImVec2 content_size = io.DisplaySize;

    ImGui::SetNextWindowSize(ImVec2(content_size.x, h));
    ImGui::SetNextWindowPos(ImVec2(0, content_size.y - h));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoInputs;

    bool open_ptr = true;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
    ImGui::Begin(u8"Консоль отладки", &open_ptr, window_flags);
    ImGui::PopStyleColor();
    ImGui::Text(u8"%s", debugMsg.toStdString().c_str());
    ImGui::End();
}

//------------------------------------------------------------------------------
// Диагностика составов (ТЗ "Промт статистики вагонов"):
// свёрнутый режим - сводка по составам (F4 переключает режимы)
//------------------------------------------------------------------------------
void MyGui::showDiagnostics() const
{
    if (!params->vehicles_handler)
    {
        return;
    }

    const simulator_diagnostics_update_t diag =
            params->vehicles_handler->getDiagnostics();

    ImGuiIO &io = ImGui::GetIO();
    ImVec2 content_size = io.DisplaySize;

    // Свёрнутый режим: только сводка составов
    const int rows = static_cast<int>(diag.trains.size()) +
            (params->diagnostics_full_mode ? 2 : 1);

    const float h = std::min(font_size * (rows + 8),
                             content_size.y * 0.8f);
    const float w = params->diagnostics_full_mode ? 980.0f : 640.0f;

    ImGui::SetNextWindowSize(ImVec2(w, h));
    ImGui::SetNextWindowPos(ImVec2(content_size.x - w, 0));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;

    bool open_ptr = true;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
    ImGui::Begin(params->diagnostics_full_mode
                     ? u8"Диагностика вагонов (F3 - закрыть, F4 - сводка)"
                     : u8"Сводка составов (F3 - закрыть, F4 - вагоны)",
                 &open_ptr, window_flags);
    ImGui::PopStyleColor();

    // --- Сводка составов ---
    if (diag.trains.empty())
    {
        ImGui::TextUnformatted(u8"Нет данных (симулятор не подключён)");
        ImGui::End();
        return;
    }

    QString header = QString("%1 %2 %3 %4 %5 %6 %7")
            .arg(u8"Поезд", 16)
            .arg(u8"ПЕ", 9)
            .arg(u8"Масса,т", 9)
            .arg(u8"Длина,м", 8)
            .arg(u8"Растяж,кН", 10)
            .arg(u8"Сжатие,кН", 10)
            .arg(u8"Сцепки:-/+");
    ImGui::TextUnformatted(header.toStdString().c_str());

    for (const auto& train : diag.trains)
    {
        QString name = train.train_name.isEmpty()
                ? QString::number(train.first_vehicle_id) : train.train_name;
        if (name.length() > 15)
        {
            name = name.left(14) + ".";
        }

        QString line = QString("%1 %2 %3 %4 %5 %6 %7/%8")
                .arg(name, 16)
                .arg(QString("%1-%2").arg(train.first_vehicle_id)
                                        .arg(train.last_vehicle_id), 9)
                .arg(train.train_mass_t, 9, 'f', 0)
                .arg(train.train_length_m, 8, 'f', 0)
                .arg(train.max_tension_kn, 10, 'f', 0)
                .arg(train.max_compression_kn, 10, 'f', 0)
                .arg(train.overloaded_joints)
                .arg(train.broken_joints);

        // Разрушенные сцепки - красным, перегруженные - жёлтым
        ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        if (train.broken_joints > 0)
        {
            color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
        }
        else if (train.overloaded_joints > 0)
        {
            color = ImVec4(1.0f, 0.8f, 0.3f, 1.0f);
        }

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(line.toStdString().c_str());
        ImGui::PopStyleColor();
    }

    // --- Полный режим: по каждому вагону ---
    if (params->diagnostics_full_mode)
    {
        ImGui::Spacing();

        QString veh_header = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11")
                .arg(u8"ПЕ", 4)
                .arg(u8"Масса,т", 8)
                .arg(u8"V,км/ч", 8)
                .arg(u8"Пикет,м", 9)
                .arg(u8"Уклон", 6)
                .arg(u8"F,кН", 7)
                .arg(u8"Кузов", 6)
                .arg(u8"Тележ.", 7)
                .arg(u8"Колодки", 8)
                .arg(u8"Уск.в/п", 9)
                .arg(u8"Прочее");
        ImGui::TextUnformatted(veh_header.toStdString().c_str());

        // Прокручиваемая область для длинных составов
        ImGui::BeginChild("vehicles", ImVec2(0.0f, 0.0f), false);

        for (const auto& vehicle : diag.vehicles)
        {
            QString flags = "";
            if (vehicle.derailed != 0)
            {
                flags += u8"СХОД ";
            }
            if (vehicle.coupled_fwd == 0)
            {
                flags += u8"| перед ";
            }
            if (vehicle.coupled_bwd == 0)
            {
                flags += u8"| зад ";
            }
            if (flags.isEmpty())
            {
                flags = u8"ок";
            }

            const QString line = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11")
                    .arg(vehicle.vehicle_idx, 4)
                    .arg(vehicle.mass_t, 8, 'f', 1)
                    .arg(vehicle.speed_kmh, 8, 'f', 1)
                    .arg(vehicle.rail_coord_m, 9, 'f', 0)
                    .arg(vehicle.inclination, 6, 'f', 1)
                    .arg(vehicle.force_kn, 7, 'f', 0)
                    .arg(vehicle.body_damage, 6, 'f', 2)
                    .arg(vehicle.bogie_damage, 7, 'f', 2)
                    .arg(QString::number(static_cast<int>(vehicle.shoe_temperature)) +
                         u8"°/" + QString::number(vehicle.brake_efficiency, 'f', 2), 8)
                    .arg(QString("%1/%2").arg(vehicle.vertical_accel, 0, 'f', 1)
                                           .arg(vehicle.lateral_accel, 0, 'f', 1), 9)
                    .arg(flags);

            // Сход - красным, повреждения - жёлтым
            ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            if (vehicle.derailed != 0)
            {
                color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
            }
            else if ((vehicle.body_damage > 0.05f) || (vehicle.bogie_damage > 0.05f))
            {
                color = ImVec4(1.0f, 0.8f, 0.3f, 1.0f);
            }

            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(line.toStdString().c_str());
            ImGui::PopStyleColor();
        }

        ImGui::EndChild();
    }

    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::drawWalkHint() const
{
    if (params->walk_hint.isEmpty())
        return;

    const std::string text = params->walk_hint.toStdString();
    const char* c_text = text.c_str();

    ImVec2 text_size = ImGui::CalcTextSize(c_text);

    ImGuiIO& io = ImGui::GetIO();
    const float w = text_size.x + 30.0f;
    const float h = text_size.y + 14.0f;

    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - w) / 2.0f,
                                   io.DisplaySize.y * 0.62f));
    ImGui::SetNextWindowSize(ImVec2(w, h));

    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoResize;
    flags |= ImGuiWindowFlags_NoCollapse;
    flags |= ImGuiWindowFlags_NoInputs;
    flags |= ImGuiWindowFlags_NoFocusOnAppearing;

    bool open_ptr = true;

    ImGui::PushStyleColor(ImGuiCol_WindowBg,
                          ImVec4(0.05f, 0.25f, 0.10f, 0.85f));
    ImGui::Begin(u8"Подсказка", &open_ptr, flags);
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 1.0f, 0.9f, 1.0f));
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - text_size.x) * 0.5f);
    ImGui::TextUnformatted(c_text);
    ImGui::PopStyleColor();
    ImGui::End();
}

void MyGui::showCassetteNotice() const
{
    if (!params->vehicles_handler)
    {
        return;
    }

    const quint32 notice_id = params->vehicles_handler->getCassetteNoticeId();

    if (notice_id == 0 || notice_id == prev_cassette_notice_id)
    {
        // Показ ещё активного предупреждения (таймер затухания)
        if (notice_shown_until > ImGui::GetTime() && !cassette_notice_text.isEmpty())
        {
            drawCassetteNotice(cassette_notice_text);
        }
        return;
    }

    // Новое предупреждение от симулятора
    prev_cassette_notice_id = notice_id;
    cassette_notice_text = params->vehicles_handler->getCassetteNotice();
    notice_shown_until = ImGui::GetTime() + 5.0;

    if (!cassette_notice_text.isEmpty())
    {
        drawCassetteNotice(cassette_notice_text);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::drawCassetteNotice(const QString& text) const
{
    const std::string std_text = text.toStdString();
    const char* c_text = std_text.c_str();

    ImVec2 text_size = ImGui::CalcTextSize(c_text);

    ImGuiIO& io = ImGui::GetIO();
    const float w = text_size.x + 40.0f;
    const float h = text_size.y + 24.0f;

    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - w) / 2.0f,
                                   io.DisplaySize.y * 0.22f));
    ImGui::SetNextWindowSize(ImVec2(w, h));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoInputs;
    window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;

    bool open_ptr = true;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.55f, 0.05f, 0.05f, 0.85f));
    ImGui::Begin(u8"Предупреждение", &open_ptr, window_flags);
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    // Текст по центру строки
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - text_size.x) * 0.5f);
    ImGui::TextUnformatted(c_text);

    ImGui::PopStyleColor();
    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::showNoControlled() const
{
    const char *text = "Нажмите Enter для управления данной ПЕ";    ImVec2 text_size = ImGui::CalcTextSize(text);

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(text_size.x + 20, text_size.y + 20));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoInputs;

    bool open_ptr = true;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
    ImGui::Begin(u8"Состояние управления", &open_ptr, window_flags);
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
    ImGui::Text(u8"%s", text);
    ImGui::PopStyleColor();
    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::showNoCabineControl() const
{
    VehicleExterior* cur = params->vehicles_handler
                           ? params->vehicles_handler->getCurrentVehicle()
                           : nullptr;
    if (!cur) return;

    std::string msg = QString("Нажмите Enter для управления из кабины %1")
                            .arg(cur->current_cabine_idx + 1)
                            .toStdString();
    const char *text = msg.c_str();
    ImVec2 text_size = ImGui::CalcTextSize(text);

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(text_size.x + 20, text_size.y + 20));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoInputs;

    bool open_ptr = true;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
    ImGui::Begin(u8"Состояние управления", &open_ptr, window_flags);
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
    ImGui::Text(u8"%s", text);
    ImGui::PopStyleColor();
    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::showTrainRenameDialog() const
{
    static char train_name[256] = "";

    int w = 300;
    int h = 70;

    ImGui::SetNextWindowSize(ImVec2(w, h));

    ImGuiIO &io = ImGui::GetIO();

    ImVec2 content_size = io.DisplaySize;

    ImGui::SetNextWindowPos(ImVec2( (content_size.x - w) / 2, (content_size.y - h) / 2));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;

    bool open_ptr = true;

    ImGui::Begin(u8"Задать имя поезда", &open_ptr, window_flags);

    float tw = 280;
    float offset_x = (w - tw) / 2.0f;

    ImGui::SetCursorPosX(offset_x);
    ImGui::SetNextItemWidth(tw);
    ImGui::SetKeyboardFocusHere();

    if (ImGui::InputText(u8"", train_name, sizeof(train_name)))
    {
        //params->is_show_trane_rename_dialog = false;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Enter))
    {
        params->is_show_trane_rename_dialog = false;

        if (params->tcp_client != nullptr)
        {
            // Здесь отправим данные серверу
            params->tcp_client->sendNewTrainName(params->vehicles_handler->getCurrentTrainIndex(), QString(train_name));
        }
    }

    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::showPauseState() const
{
    QString text = QString("ПAУЗА\nДля возобновления игры нажмите Pause");

    ImGuiIO &io = ImGui::GetIO();
    ImVec2 content_size = io.DisplaySize;

    // Предварительный расчёт габаритов текста для центрирования окна
    ImVec2 text_size = ImGui::CalcTextSize(text.toStdString().c_str());

    // Центрируем окно по середине экрана
    ImGui::SetNextWindowPos(ImVec2(
        (content_size.x - text_size.x) * 0.5f,
        (content_size.y - text_size.y) * 0.5f
        ));
    ImGui::SetNextWindowSize(ImVec2(text_size.x + 20, text_size.y + 20));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoInputs;

    bool open_ptr = true;

    // Полупрозрачный фон для лучшей читаемости
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
    ImGui::Begin(u8"Пауза", &open_ptr, window_flags);
    ImGui::PopStyleColor();

    // Красный цвет текста
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

    // Построчное центрирование текста
    QStringList lines = text.split('\n');
    for (const QString &line : lines)
    {
        QByteArray line_utf8 = line.toUtf8();
        ImVec2 line_size = ImGui::CalcTextSize(line_utf8.constData());
        float cursor_x = (ImGui::GetWindowWidth() - line_size.x) * 0.5f;
        ImGui::SetCursorPosX(cursor_x);
        ImGui::Text("%s", line_utf8.constData());
    }

    ImGui::PopStyleColor();
    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::showHUD() const
{
    showTimetable();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::showTimetable() const
{
    // Получаем данные о графике
    autopilot_timetable_t timetable = params->vehicles_handler->getTimetable();

    // Если данные пустые - на выход
    if (timetable.stations.empty())
    {
        return;
    }

    // Число строк в таблице графика
    size_t rows_count = timetable.stations.size() - timetable.start_station_idx;

    // Высота окна
    float lineHeightWithSpacing = ImGui::GetTextLineHeightWithSpacing();
    float h = lineHeightWithSpacing * (rows_count + 3);
    // Ширина окна
    float w = 500.0;

    ImGui::SetNextWindowSize(ImVec2(w, h));
    ImGui::SetNextWindowPos(ImVec2(0.0, 300.0));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoInputs;

    bool open_ptr = true;

    QString title = QString("Поезд %1 %2 %3 %4 %5").arg(timetable.train_name.leftJustified(9))
                        .arg(QString("Приб.").leftJustified(5))
                        .arg(QString("Отпр.").leftJustified(5))
                        .arg(QString("Факт. приб.").leftJustified(10))
                        .arg(QString("Факт. отпр.").leftJustified(10));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
    ImGui::Begin(u8"Нормативный график", &open_ptr, window_flags);
    ImGui::PopStyleColor();

    QString time = QString("Время: %1 Дист. до цели: %2 м")
                       .arg(params->sim_time->time.getString(), 8)
                       .arg(timetable.target_station_dist, 7, 'f', 1);

    ImGui::Text(u8"%s", time.toStdString().c_str());
    ImGui::Text(u8"%s", title.toStdString().c_str());


    for (int i = timetable.start_station_idx; i < timetable.stations.size(); ++i)
    {
        if (!timetable.stations[i].is_visible)
        {
            continue;
        }

        const int NAME_SIZE = 15;

        QString striped_name = timetable.stations[i].name;

        if (striped_name.length() > NAME_SIZE)
        {
            striped_name = striped_name.left(NAME_SIZE - 1) + ".";
        }

        QString station_info = QString("%1 %2 %3 %4 %5")
                                   .arg(striped_name.leftJustified(NAME_SIZE))
                                   .arg(timetable.stations[i].arr_time, 5)
                                   .arg(timetable.stations[i].dep_time, 5)
                                   .arg(timetable.stations[i].fact_arr_time, 10)
                                   .arg(timetable.stations[i].fact_dep_time, 10);

        if (i < timetable.stations.size() - 1)
        {
            station_info += "\n";
        }

        ImVec4 textColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

        if (timetable.stations[i].arr_delay || timetable.stations[i].dep_delay)
        {
            textColor = ImVec4(1.0f, 0.5f, 0.31f, 1.0f);
        }
        else
        {
            if (i < timetable.curr_station_idx)
            {
                textColor = ImVec4(0.0f, 0.5f, 0.0f, 1.0f);
            }
            else
            {
                textColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
            }
        }

        if (i == timetable.curr_station_idx)
        {
            textColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        }

        ImGui::PushStyleColor(ImGuiCol_Text, textColor);
        ImGui::Text(u8"%s", station_info.toStdString().c_str());
        ImGui::PopStyleColor();
    }

    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::check_date_time() const
{
    params->msec = 0;

    if (params->sec > 59)
    {
        ++params->minute;
        params->sec = 0;
    }
    else if (params->sec < 0)
    {
        --params->minute;
        params->sec = 59;
    }

    if (params->minute > 59)
    {
        ++params->hour;
        params->minute = 0;
    }
    else if (params->minute < 0)
    {
        --params->hour;
        params->minute = 59;
    }

    if (params->hour > 23)
    {
        ++params->day;
        params->hour = 0;
    }
    else if (params->hour < 0)
    {
        --params->day;
        params->hour = 23;
    }

    if (params->day > (server_date_t::isLeapYear(params->year) ?
                           days_in_month_leap[std::clamp(params->month, int16_t(1), int16_t(12)) - 1] :
                           days_in_month_nleap[std::clamp(params->month, int16_t(1), int16_t(12)) - 1]))
    {
        ++params->month;
        params->day = 1;
    }
    else if (params->day < 1)
    {
        --params->month;
        params->day = (server_date_t::isLeapYear(params->year) ?
                           days_in_month_leap[std::clamp(params->month, int16_t(1), int16_t(12)) - 1] :
                           days_in_month_nleap[std::clamp(params->month, int16_t(1), int16_t(12)) - 1]);
    }

    if (params->month > 12)
    {
        ++params->year;
        params->month = 1;
    }
    else if (params->month < 1)
    {
        --params->year;
        params->month = 12;
    }
}

//------------------------------------------------------------------------------
// Подсказка органа кабины (Alt): имя, назначение, состояние
//------------------------------------------------------------------------------
void MyGui::showCabTooltip() const
{
    const CabTooltipState& tip = cabTooltip();

    if (!tip.active)
    {
        return;
    }

    const float pad = 12.0f;
    const ImVec2 pos(tip.x + pad, tip.y + pad);

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.85f);

    const int flags = ImGuiWindowFlags_NoTitleBar |
                      ImGuiWindowFlags_NoResize |
                      ImGuiWindowFlags_NoMove |
                      ImGuiWindowFlags_NoCollapse |
                      ImGuiWindowFlags_AlwaysAutoResize |
                      ImGuiWindowFlags_NoSavedSettings |
                      ImGuiWindowFlags_NoNav |
                      ImGuiWindowFlags_NoFocusOnAppearing;

    if (ImGui::Begin("##cab_tooltip", nullptr, flags))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.4f, 1.0f));
        ImGui::TextUnformatted(tip.title.c_str());
        ImGui::PopStyleColor();

        // Только название прибора + что произойдёт при нажатии:
        // расширенные пояснения (Hint) убраны как неактуальные
        if (!tip.state_text.empty())
        {
            ImGui::TextDisabled("%s", tip.state_text.c_str());
        }

        if (!tip.action_text.empty())
        {
            ImGui::TextDisabled("%s", tip.action_text.c_str());
        }
    }

    ImGui::End();
}
