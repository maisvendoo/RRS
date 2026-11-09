#include "MyGui.h"

#include "filesystem.h"
#include "datetime.h"

#include "NewSkybox.h"
#include "RouteViewer.h"
// #include "Skybox.h"
#include "Sun.h"
#include "UpdateStatisticsHandler.h"
#include "UpdateControlToServerHandler.h"
#include "VehiclesHandler.h"
#include "UpdateViewerHandler.h"
#include "StationsHandler.h"
#include "TrainLabelsHandler.h"
#include "TrafficLightsHandler.h"
#include "TrafficLight.h"
#include "MouseControlHandler.h"
#include <tcp-client.h>

#include "vehicle-service-command.h"

#include <QDataStream>

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
#include <cmath>

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

    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    _trains_list_params.vehicles_handler = params->vehicles_handler;
    _trains_list_params.viewer_handler = params->viewer_handler;
    _trains_list_params.tcp_client = params->tcp_client;
    _trains_list_params.hud_background = params->hud_background;
    _trains_list_params.hud_text = params->hud_text;
    _trains_list_params.hud_current_train = params->hud_current_train;
    _trains_list_params.hud_controlled_train = params->hud_controlled_train;
    _trains_list_widget = new TrainsListWidget(&_trains_list_params);

    _train_profile_params.vehicles_handler = params->vehicles_handler;
    _train_profile_params.traffic_lights_handler = params->traffic_lights_handler;
    _train_profile_params.backward_m = static_cast<float>(params->train_profile_backward);
    _train_profile_params.forward_m = static_cast<float>(params->train_profile_forward);
    _train_profile_params.hud_background = params->hud_background;
    _train_profile_params.hud_train_profile_grid = params->hud_train_profile_grid;
    _train_profile_params.hud_train_profile_grid_label = params->hud_train_profile_grid_label;
    _train_profile_params.hud_train_profile_baseline = params->hud_train_profile_baseline;
    _train_profile_params.hud_train_profile_curve = params->hud_train_profile_curve;
    _train_profile_params.hud_train_profile_uncontrolled = params->hud_train_profile_uncontrolled;
    _train_profile_params.hud_train_profile_current = params->hud_train_profile_current;
    _train_profile_params.hud_train_profile_controlled = params->hud_train_profile_controlled;
    _train_profile_params.hud_train_profile_station_text = params->hud_train_profile_station_text;
    _train_profile_params.hud_train_profile_signal_body = params->hud_train_profile_signal_body;
    _train_profile_params.hud_train_profile_signal_letter = params->hud_train_profile_signal_letter;
    _train_profile_params.hud_train_profile_speed_limit_border = params->hud_train_profile_speed_limit_border;
    _train_profile_params.hud_train_profile_speed_limit_fill = params->hud_train_profile_speed_limit_fill;
    _train_profile_params.hud_train_profile_speed_limit_text = params->hud_train_profile_speed_limit_text;
    _train_profile_params.hud_train_profile_speed_limit_bg = params->hud_train_profile_speed_limit_bg;
    _train_profile_widget = new TrainProfileHintWidget(&_train_profile_params);
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

    ImGuiIO &io = ImGui::GetIO();

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

    // F8: Ctrl+F8 - переименование поезда, Alt+F8 - HUD.
    // Обычный F8 - режим машиниста пешком (обрабатывает UpdateViewerHandler).
    const bool f8_pressed = ImGui::IsKeyPressed(ImGuiKey_F8);
    if (f8_pressed && !params->prev_F8)
    {
        if (io.KeyCtrl)
        {
            params->is_show_trane_rename_dialog = !params->is_show_trane_rename_dialog;
        }
        else if (io.KeyAlt)
        {
            params->is_show_HUD = !params->is_show_HUD;
        }
    }
    params->prev_F8 = f8_pressed;

    if (params->is_show_trane_rename_dialog)
    {
        showTrainRenameDialog();
    }

    // F5 - информация о вагоне (листание по составу),
    // Ctrl+F5 - ограничения скорости
    const bool f5_pressed = ImGui::IsKeyPressed(ImGuiKey_F5);
    if (f5_pressed && !params->prev_F5)
    {
        if (io.KeyCtrl)
        {
            params->is_show_speed_limits = !params->is_show_speed_limits;
        }
        else if (!io.KeyShift && !io.KeyAlt)
        {
            if (!params->is_show_car_info)
            {
                params->is_show_car_info = true;
                params->car_info_page = 0;
            }
            else
            {
                ++params->car_info_page;
            }
        }
    }
    params->prev_F5 = f5_pressed;

    // F6 - станции впереди
    const bool f6_pressed = ImGui::IsKeyPressed(ImGuiKey_F6);
    if (f6_pressed && !params->prev_F6 && !io.KeyCtrl && !io.KeyShift && !io.KeyAlt)
    {
        params->is_show_stations_panel = !params->is_show_stations_panel;
    }
    params->prev_F6 = f6_pressed;

    // F7 - подписи станций/поездов, Ctrl+F7 - светофоры
    const bool f7_pressed = ImGui::IsKeyPressed(ImGuiKey_F7);
    if (f7_pressed && !params->prev_F7)
    {
        if (io.KeyCtrl)
        {
            params->is_show_signal_panel = !params->is_show_signal_panel;
        }
        else if (!is_modified_key)
        {
            params->is_show_labels = !params->is_show_labels;
        }
    }
    params->prev_F7 = f7_pressed;

    if (params->is_show_HUD)
    {
        showHUD();
    }

    if (params->is_show_car_info)
    {
        showCarInfoPanel();
    }
    if (params->is_show_speed_limits)
    {
        showSpeedLimitsPanel();
    }
    if (params->is_show_stations_panel)
    {
        showStationsPanel();
    }
    if (params->is_show_signal_panel)
    {
        showSignalsPanel();
    }

    applyWorldLabels();

    // Подсказка органа кабины под курсором (Alt удерживается)

    // Диагностика составов:
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

    // Меню вагона: Num3 - открыть/закрыть окно сервисных команд
    if (ImGui::IsKeyPressed(ImGuiKey_Keypad3) && !params->prev_Num3 && !is_modified_key)
    {
        params->is_show_car_menu = !params->is_show_car_menu;

        if (params->is_show_car_menu)
        {
            // При открытии заново перечитываем состояние вагона
            params->car_menu_vehicle = -1;
        }
    }
    params->prev_Num3 = ImGui::IsKeyPressed(ImGuiKey_Keypad3);

    if (params->is_show_car_menu)
    {
        showCarMenu();
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

        // F9 - дебаг-строка подвижного состава, Ctrl+F9 - язык интерфейса
        const bool f9_pressed = ImGui::IsKeyPressed(ImGuiKey_F9);
        if (f9_pressed && !params->prev_F9)
        {
            if (io.KeyCtrl)
            {
                params->use_english = !params->use_english;
            }
            else if (!is_modified_key)
            {
                params->is_show_debug_msg = !params->is_show_debug_msg;
                params->controls_handler->setNeedDebugMsg(params->is_show_debug_msg);
            }
        }
        params->prev_F9 = f9_pressed;

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

    // Предупреждение кассеты регистрации: всплывает
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

    showControlTooltip();
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

    ImGui::PushStyleColor(ImGuiCol_WindowBg, params->hud_background);
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

    ImGui::PushStyleColor(ImGuiCol_WindowBg, params->hud_background);
    ImGui::Begin(u8"Статистика", &open_ptr, window_flags);
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text, params->hud_text);
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

    int language = params->use_english ? 1 : 0;
    const char* language_items[] = { u8"Русский", "English" };
    if (ImGui::Combo(u8"Язык / Language", &language, language_items, 2))
    {
        params->use_english = (language == 1);
    }

    ImGui::TextWrapped("%s", tr(u8"F5 - вагон, Ctrl+F5 - ограничения, F6 - станции, "
                                 u8"F7 - подписи, Ctrl+F7 - светофоры, F8 - пешком, "
                                 u8"Alt+F8 - HUD, Ctrl+F8 - переименование",
                                 "F5 - car, Ctrl+F5 - limits, F6 - stations, "
                                 "F7 - labels, Ctrl+F7 - signals, F8 - walk, "
                                 "Alt+F8 - HUD, Ctrl+F8 - rename").toUtf8().constData());

    ImGui::TextWrapped("%s", tr(u8"В кабине: F8 - встать, Enter - сесть, "
                                 u8"Ctrl+Enter - выйти на землю, Num0 - окно, "
                                 u8"Num1 - высунуться",
                                 "In cab: F8 - stand up, Enter - sit down, "
                                 "Ctrl+Enter - step outside, Num0 - window, "
                                 "Num1 - lean out").toUtf8().constData());
    ImGui::Separator();

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
// Выбор пресета графики: применяется на лету всё, что
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

    // Статусы тиров нового качества: PBR и ACES-тонмаппинг
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

    // Пост-процесс пресета Extreme: Bloom/SSAO/SSR/Туман —
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

    ImGui::PushStyleColor(ImGuiCol_WindowBg, params->hud_background);
    ImGui::Begin(u8"Консоль отладки", &open_ptr, window_flags);
    ImGui::PopStyleColor();
    ImGui::Text(u8"%s", debugMsg.toStdString().c_str());
    ImGui::End();
}

//------------------------------------------------------------------------------
// Диагностика составов:
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

    ImGui::PushStyleColor(ImGuiCol_WindowBg, params->hud_background);
    ImGui::Begin(u8"Состояние управления ПЕ", &open_ptr, window_flags);
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text, params->hud_warning_text);
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

    ImGui::PushStyleColor(ImGuiCol_WindowBg, params->hud_background);
    ImGui::Begin(u8"Состояние управления кабиной", &open_ptr, window_flags);
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text, params->hud_warning_text);
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
QString MyGui::tr(const char* ru, const char* en) const
{
    return QString::fromUtf8(params->use_english ? en : ru);
}

//------------------------------------------------------------------------------
// Информация о вагоне (F5), листается по составу
//------------------------------------------------------------------------------
void MyGui::showCarInfoPanel() const
{
    bool open = params->is_show_car_info;
    QByteArray title = tr(u8"Информация о вагоне", "Car information").toUtf8();

    ImGui::SetNextWindowSize(ImVec2(460, 380), ImGuiCond_FirstUseEver);

    if (ImGui::Begin(title.constData(), &open))
    {
        if (params->vehicles_handler == nullptr)
        {
            ImGui::TextUnformatted(tr(u8"Нет данных", "No data").toUtf8().constData());
        }
        else
        {
            const auto& trains = params->vehicles_handler->getTrainsInfo();
            const int train_idx = params->vehicles_handler->getCurrentTrainIndex();
            const auto diag = params->vehicles_handler->getDiagnostics();

            if (train_idx < 0 || train_idx >= static_cast<int>(trains.size()))
            {
                ImGui::TextUnformatted(tr(u8"Нет данных", "No data").toUtf8().constData());
            }
            else
            {
                const int lo = std::min(trains[train_idx].first_vehicle_id,
                                        trains[train_idx].last_vehicle_id);
                const int hi = std::max(trains[train_idx].first_vehicle_id,
                                        trains[train_idx].last_vehicle_id);

                std::vector<const simulator_vehicle_diagnostics_t*> cars;
                for (const auto& v : diag.vehicles)
                {
                    if (v.vehicle_idx >= lo && v.vehicle_idx <= hi)
                    {
                        cars.push_back(&v);
                    }
                }

                std::sort(cars.begin(), cars.end(),
                          [](const auto* a, const auto* b)
                {
                    return a->vehicle_idx < b->vehicle_idx;
                });

                if (cars.empty())
                {
                    ImGui::TextUnformatted(tr(u8"Нет данных", "No data").toUtf8().constData());
                }
                else
                {
                    int page = params->car_info_page % static_cast<int>(cars.size());
                    if (page < 0)
                    {
                        page += static_cast<int>(cars.size());
                    }

                    const auto* car = cars[page];

                    ImGui::Text("%s: %s",
                                tr(u8"Поезд", "Train").toUtf8().constData(),
                                trains[train_idx].train_name.toUtf8().constData());
                    ImGui::Text("%s: %d / %d",
                                tr(u8"Вагон", "Car").toUtf8().constData(),
                                page + 1,
                                static_cast<int>(cars.size()));

                    QString cfg_name;
                    const auto& info = params->vehicles_handler->getVehiclesInfo().vehicles;
                    if (car->vehicle_idx >= 0
                        && car->vehicle_idx < static_cast<int>(info.size()))
                    {
                        cfg_name = info[car->vehicle_idx].vehicle_config_file;
                        cfg_name = cfg_name.section('/', -1).section('\\', -1);
                        if (cfg_name.endsWith(".xml", Qt::CaseInsensitive))
                        {
                            cfg_name.chop(4);
                        }
                    }

                    if (!cfg_name.isEmpty())
                    {
                        ImGui::Text("%s: %s",
                                    tr(u8"Модель", "Model").toUtf8().constData(),
                                    cfg_name.toUtf8().constData());
                    }

                    ImGui::Separator();
                    ImGui::Text("%s: %.2f %s",
                                tr(u8"Масса", "Mass").toUtf8().constData(),
                                car->mass_t,
                                tr(u8"т", "t").toUtf8().constData());
                    ImGui::Text("%s: %.1f %s",
                                tr(u8"Скорость", "Speed").toUtf8().constData(),
                                car->speed_kmh,
                                tr(u8"км/ч", "km/h").toUtf8().constData());
                    ImGui::Text("%s: %.0f %s",
                                tr(u8"Пикетаж", "Railway coordinate").toUtf8().constData(),
                                car->rail_coord_m,
                                tr(u8"м", "m").toUtf8().constData());
                    ImGui::Text("%s: %.1f %s",
                                tr(u8"Уклон", "Gradient").toUtf8().constData(),
                                car->inclination,
                                tr(u8"тыс.", "thousandths").toUtf8().constData());

                    ImGui::Separator();
                    ImGui::Text("%s: %s",
                                tr(u8"Сход", "Derailed").toUtf8().constData(),
                                car->derailed ? tr(u8"да", "yes").toUtf8().constData()
                                              : tr(u8"нет", "no").toUtf8().constData());
                    ImGui::Text("%s: %s / %s",
                                tr(u8"Сцеплен", "Coupled").toUtf8().constData(),
                                car->coupled_fwd ? tr(u8"спереди", "front").toUtf8().constData()
                                                 : tr(u8"нет", "no").toUtf8().constData(),
                                car->coupled_bwd ? tr(u8"сзади", "rear").toUtf8().constData()
                                                 : tr(u8"нет", "no").toUtf8().constData());
                    ImGui::Text("%s: %.0f%% / %.0f%%",
                                tr(u8"Кузов/тележка", "Body/bogie").toUtf8().constData(),
                                car->body_damage * 100.0f,
                                car->bogie_damage * 100.0f);
                    ImGui::Text("%s: %.0f%%",
                                tr(u8"Тормоза", "Brakes").toUtf8().constData(),
                                car->brake_efficiency * 100.0f);
                    ImGui::Text("%s: %.0f C",
                                tr(u8"Температура колодок", "Shoe temperature").toUtf8().constData(),
                                car->shoe_temperature);

                    ImGui::Separator();
                    if (ImGui::Button("<<"))
                    {
                        --params->car_info_page;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button(">>"))
                    {
                        ++params->car_info_page;
                    }
                    ImGui::SameLine();
                    ImGui::TextUnformatted(tr(u8"F5 - следующий вагон",
                                              "F5 - next car").toUtf8().constData());
                }
            }
        }
    }

    ImGui::End();
    params->is_show_car_info = open;
}

//------------------------------------------------------------------------------
// Ограничения скорости (Ctrl+F5)
//------------------------------------------------------------------------------
void MyGui::showSpeedLimitsPanel() const
{
    bool open = params->is_show_speed_limits;
    QByteArray title = tr(u8"Ограничения скорости", "Speed limits").toUtf8();

    ImGui::SetNextWindowSize(ImVec2(430, 360), ImGuiCond_FirstUseEver);

    if (ImGui::Begin(title.constData(), &open))
    {
        if (params->vehicles_handler == nullptr)
        {
            ImGui::TextUnformatted(tr(u8"Нет данных", "No data").toUtf8().constData());
        }
        else
        {
            simulator_train_profile_update_t profile;
            const int train_idx = params->vehicles_handler->getCurrentTrainIndex();

            if (!params->vehicles_handler->getTrainProfile(train_idx, profile))
            {
                ImGui::TextUnformatted(tr(u8"Нет данных", "No data").toUtf8().constData());
            }
            else
            {
                const float dir = (profile.direction < 0) ? -1.0f : 1.0f;

                float nearest_point = 1.0e9f;
                float railway_coord = 0.0f;
                for (const auto& point : profile.profile)
                {
                    const float d = std::fabs(point.distance);
                    if (d < nearest_point)
                    {
                        nearest_point = d;
                        railway_coord = point.railway_coord;
                    }
                }

                bool has_limit = false;
                float current_limit = 0.0f;
                for (const auto& limit : profile.speed_limits)
                {
                    const bool covers_train =
                        (limit.distance <= 0.0f && limit.end_distance >= 0.0f) ||
                        (limit.end_distance <= 0.0f && limit.distance >= 0.0f);

                    if (covers_train)
                    {
                        if (!has_limit || limit.speed_kmh < current_limit)
                        {
                            current_limit = limit.speed_kmh;
                            has_limit = true;
                        }
                    }
                }

                if (!has_limit)
                {
                    float next_distance = 1.0e9f;
                    for (const auto& limit : profile.speed_limits)
                    {
                        const float d = limit.distance * dir;
                        if (d >= -1.0f && d < next_distance)
                        {
                            next_distance = d;
                            current_limit = limit.speed_kmh;
                            has_limit = true;
                        }
                    }
                }

                ImGui::Text("%s: %.1f %s",
                            tr(u8"Скорость", "Speed").toUtf8().constData(),
                            profile.speed * 3.6f,
                            tr(u8"км/ч", "km/h").toUtf8().constData());
                ImGui::Text("%s: %.0f %s",
                            tr(u8"Пикетаж", "Railway coordinate").toUtf8().constData(),
                            railway_coord,
                            tr(u8"м", "m").toUtf8().constData());
                ImGui::Text("%s: %s",
                            tr(u8"Текущее ограничение", "Current limit").toUtf8().constData(),
                            has_limit ? QString::number(current_limit, 'f', 1).toUtf8().constData()
                                      : tr(u8"нет данных", "no data").toUtf8().constData());

                ImGui::Separator();
                ImGui::TextUnformatted(tr(u8"Впереди:", "Ahead:").toUtf8().constData());

                std::vector<const simulator_train_profile_speed_limit_t*> ahead;
                for (const auto& limit : profile.speed_limits)
                {
                    const float d = limit.distance * dir;
                    if (d > 1.0f)
                    {
                        ahead.push_back(&limit);
                    }
                }

                std::sort(ahead.begin(), ahead.end(),
                          [dir](const auto* a, const auto* b)
                {
                    return a->distance * dir < b->distance * dir;
                });

                if (ahead.empty())
                {
                    ImGui::TextUnformatted(tr(u8"нет данных", "no data").toUtf8().constData());
                }
                else
                {
                    const size_t count = std::min<size_t>(ahead.size(), 8);
                    for (size_t i = 0; i < count; ++i)
                    {
                        const float d = ahead[i]->distance * dir;
                        ImGui::Text("+%.0f %s  %.0f %s",
                                    d,
                                    tr(u8"м", "m").toUtf8().constData(),
                                    ahead[i]->speed_kmh,
                                    tr(u8"км/ч", "km/h").toUtf8().constData());
                    }
                }
            }
        }
    }

    ImGui::End();
    params->is_show_speed_limits = open;
}

//------------------------------------------------------------------------------
// Станции впереди (F6)
//------------------------------------------------------------------------------
void MyGui::showStationsPanel() const
{
    bool open = params->is_show_stations_panel;
    QByteArray title = tr(u8"Станции впереди", "Stations ahead").toUtf8();

    ImGui::SetNextWindowSize(ImVec2(420, 320), ImGuiCond_FirstUseEver);

    if (ImGui::Begin(title.constData(), &open))
    {
        if (params->vehicles_handler == nullptr)
        {
            ImGui::TextUnformatted(tr(u8"Нет данных", "No data").toUtf8().constData());
        }
        else
        {
            simulator_train_profile_update_t profile;
            const int train_idx = params->vehicles_handler->getCurrentTrainIndex();

            if (!params->vehicles_handler->getTrainProfile(train_idx, profile))
            {
                ImGui::TextUnformatted(tr(u8"Нет данных", "No data").toUtf8().constData());
            }
            else
            {
                const float dir = (profile.direction < 0) ? -1.0f : 1.0f;

                std::vector<const simulator_train_profile_station_t*> ahead;
                for (const auto& station : profile.stations)
                {
                    if (station.distance * dir >= -1.0f)
                    {
                        ahead.push_back(&station);
                    }
                }

                std::sort(ahead.begin(), ahead.end(),
                          [dir](const auto* a, const auto* b)
                {
                    return a->distance * dir < b->distance * dir;
                });

                if (ahead.empty())
                {
                    ImGui::TextUnformatted(tr(u8"нет данных", "no data").toUtf8().constData());
                }
                else
                {
                    const size_t count = std::min<size_t>(ahead.size(), 12);
                    for (size_t i = 0; i < count; ++i)
                    {
                        const float d = ahead[i]->distance * dir;
                        if (d < 1.0f)
                        {
                            ImGui::Text("%s: %s",
                                        tr(u8"Сейчас", "Now").toUtf8().constData(),
                                        ahead[i]->name.toUtf8().constData());
                        }
                        else
                        {
                            ImGui::Text("+%.0f %s  %s",
                                        d,
                                        tr(u8"м", "m").toUtf8().constData(),
                                        ahead[i]->name.toUtf8().constData());
                        }
                    }
                }
            }
        }
    }

    ImGui::End();
    params->is_show_stations_panel = open;
}

//------------------------------------------------------------------------------
// Светофоры впереди (Ctrl+F7)
//------------------------------------------------------------------------------
void MyGui::showSignalsPanel() const
{
    bool open = params->is_show_signal_panel;
    QByteArray title = tr(u8"Светофоры впереди", "Signals ahead").toUtf8();

    ImGui::SetNextWindowSize(ImVec2(440, 320), ImGuiCond_FirstUseEver);

    if (ImGui::Begin(title.constData(), &open))
    {
        if (params->vehicles_handler == nullptr)
        {
            ImGui::TextUnformatted(tr(u8"Нет данных", "No data").toUtf8().constData());
        }
        else
        {
            simulator_train_profile_update_t profile;
            const int train_idx = params->vehicles_handler->getCurrentTrainIndex();

            if (!params->vehicles_handler->getTrainProfile(train_idx, profile))
            {
                ImGui::TextUnformatted(tr(u8"Нет данных", "No data").toUtf8().constData());
            }
            else
            {
                const float dir = (profile.direction < 0) ? -1.0f : 1.0f;

                std::vector<const simulator_train_profile_signal_t*> ahead;
                for (const auto& signal : profile.signal_list)
                {
                    if (signal.distance * dir >= -1.0f)
                    {
                        ahead.push_back(&signal);
                    }
                }

                std::sort(ahead.begin(), ahead.end(),
                          [dir](const auto* a, const auto* b)
                {
                    return a->distance * dir < b->distance * dir;
                });

                if (ahead.empty())
                {
                    ImGui::TextUnformatted(tr(u8"нет данных", "no data").toUtf8().constData());
                }
                else
                {
                    const size_t count = std::min<size_t>(ahead.size(), 12);
                    for (size_t i = 0; i < count; ++i)
                    {
                        const auto* signal = ahead[i];
                        const float d = signal->distance * dir;

                        QString name = signal->connector_name;
                        if (params->traffic_lights_handler != nullptr)
                        {
                            const TrafficLight* light =
                                params->traffic_lights_handler->findSignal(
                                    signal->connector_name, signal->signal_dir);
                            if (light != nullptr && !light->getLetter().isEmpty())
                            {
                                name = light->getLetter();
                            }
                        }

                        const QByteArray name_utf8 = name.toUtf8();
                        if (d < 1.0f)
                        {
                            ImGui::Text("%s: %s%s",
                                        tr(u8"Сейчас", "Now").toUtf8().constData(),
                                        name_utf8.constData(),
                                        signal->is_oncoming
                                            ? tr(u8" (встречный)", " (oncoming)").toUtf8().constData()
                                            : "");
                        }
                        else
                        {
                            ImGui::Text("+%.0f %s  %s%s",
                                        d,
                                        tr(u8"м", "m").toUtf8().constData(),
                                        name_utf8.constData(),
                                        signal->is_oncoming
                                            ? tr(u8" (встречный)", " (oncoming)").toUtf8().constData()
                                            : "");
                        }
                    }
                }
            }
        }
    }

    ImGui::End();
    params->is_show_signal_panel = open;
}

//------------------------------------------------------------------------------
// Подписи станций и поездов (F7)
//------------------------------------------------------------------------------
void MyGui::applyWorldLabels() const
{
    if (params->stations_handler)
    {
        params->stations_handler->setVisible(
            params->is_show_labels && params->hud_show_stations);
    }

    if (params->train_labels_handler)
    {
        params->train_labels_handler->setVisible(
            params->is_show_labels && params->hud_show_train_labels);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::showPauseState() const
{
    QString text = params->use_english
        ? QString::fromUtf8(u8"PAUSED\nPress Pause to resume")
        : QString::fromUtf8(u8"ПАУЗА\nДля возобновления игры нажмите Pause");

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
    ImGui::PushStyleColor(ImGuiCol_WindowBg, params->hud_background);
    ImGui::Begin(u8"Пауза", &open_ptr, window_flags);
    ImGui::PopStyleColor();

    // Красный цвет текста
    ImGui::PushStyleColor(ImGuiCol_Text, params->hud_warning_text);

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
    // Тулбар с кнопками включения/выключения виджетов
    const float bar_height = hudTopOffset();
    const ImVec2 display_size = ImGui::GetIO().DisplaySize;

    const float total_w = 600.0f;

    ImGui::SetNextWindowPos(ImVec2((display_size.x - total_w) * 0.5f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(total_w, bar_height));

    ImGuiWindowFlags bar_flags = 0;
    bar_flags |= ImGuiWindowFlags_NoTitleBar;
    bar_flags |= ImGuiWindowFlags_NoResize;
    bar_flags |= ImGuiWindowFlags_NoCollapse;
    bar_flags |= ImGuiWindowFlags_NoScrollbar;

    bool open_ptr = true;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, params->hud_background);
    ImGui::Begin(u8"Панель HUD", &open_ptr, bar_flags);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImGui::PushStyleColor(ImGuiCol_Button, params->hud_button_off);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, params->hud_button_hovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, params->hud_button_on);

    float gap = 8.0f;
    float pad = gap;
    // Последняя кнопка ("Номера поездов") делается шире остальных
    float last_btn_w = 185.0f;
    float btn_w = (total_w - pad * 2 - gap * 3 - last_btn_w) / 4.0f;
    float btn_h = ImGui::GetWindowHeight() - 4.0f;

    ImGui::SetCursorPos(ImVec2(pad, 2.0f));
    ImGui::PushID("hud_profile");
    if (params->hud_show_profile)
        ImGui::PushStyleColor(ImGuiCol_Button, params->hud_button_on);
    if (ImGui::Button(tr(u8"Профиль", "Profile").toUtf8().constData(), ImVec2(btn_w, btn_h)))
        params->hud_show_profile = !params->hud_show_profile;
    if (params->hud_show_profile)
        ImGui::PopStyleColor();
    ImGui::PopID();

    ImGui::SetCursorPos(ImVec2(pad + (btn_w + gap), 2.0f));

    // Кнопка "График" — неактивна, если данных нет
    const bool has_timetable = (params->vehicles_handler)
        ? !params->vehicles_handler->getTimetable().stations.empty() : false;
    ImGui::PushID("hud_timetable");
    if (!has_timetable)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, params->hud_button_inactive);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, params->hud_button_inactive);
        ImGui::PushStyleColor(ImGuiCol_Text, params->hud_button_inactive_text);
    }
    else if (params->hud_show_timetable)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, params->hud_button_on);
    }
    if (ImGui::Button(tr(u8"График", "Timetable").toUtf8().constData(), ImVec2(btn_w, btn_h)) && has_timetable)
        params->hud_show_timetable = !params->hud_show_timetable;
    if (!has_timetable)
    {
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
    }
    else if (params->hud_show_timetable)
    {
        ImGui::PopStyleColor();
    }
    ImGui::PopID();

    ImGui::SetCursorPos(ImVec2(pad + (btn_w + gap) * 2, 2.0f));

    // Кнопка "Поезда"
    ImGui::PushID("hud_list");
    if (params->hud_show_trains_list)
        ImGui::PushStyleColor(ImGuiCol_Button, params->hud_button_on);
    if (ImGui::Button(tr(u8"Поезда", "Trains").toUtf8().constData(), ImVec2(btn_w, btn_h)))
        params->hud_show_trains_list = !params->hud_show_trains_list;
    if (params->hud_show_trains_list)
        ImGui::PopStyleColor();
    ImGui::PopID();

    ImGui::SetCursorPos(ImVec2(pad + (btn_w + gap) * 3, 2.0f));

    // Кнопка "Станции" — переключает отображение подписей станций в сцене
    const bool has_stations = (params->stations_handler != nullptr)
        && (params->stations_handler->getRootNode() != nullptr);
    ImGui::PushID("hud_stations");
    if (!has_stations)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, params->hud_button_inactive);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, params->hud_button_inactive);
        ImGui::PushStyleColor(ImGuiCol_Text, params->hud_button_inactive_text);
    }
    else if (params->hud_show_stations)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, params->hud_button_on);
    }
    if (ImGui::Button(tr(u8"Станции", "Stations").toUtf8().constData(), ImVec2(btn_w, btn_h)) && has_stations)
    {
        params->hud_show_stations = !params->hud_show_stations;
    }
    if (!has_stations)
    {
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
    }
    else if (params->hud_show_stations)
    {
        ImGui::PopStyleColor();
    }
    ImGui::PopID();

    ImGui::SetCursorPos(ImVec2(pad + (btn_w + gap) * 4, 2.0f));

    // Кнопка "Номера поездов" — переключает подписи поездов в сцене
    const bool has_train_labels = (params->train_labels_handler != nullptr)
        && (params->train_labels_handler->getRootNode() != nullptr);
    ImGui::PushID("hud_train_labels");
    if (!has_train_labels)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, params->hud_button_inactive);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, params->hud_button_inactive);
        ImGui::PushStyleColor(ImGuiCol_Text, params->hud_button_inactive_text);
    }
    else if (params->hud_show_train_labels)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, params->hud_button_on);
    }
    if (ImGui::Button(tr(u8"Номера поездов", "Train labels").toUtf8().constData(), ImVec2(last_btn_w, btn_h)) && has_train_labels)
    {
        params->hud_show_train_labels = !params->hud_show_train_labels;
    }
    if (!has_train_labels)
    {
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
    }
    else if (params->hud_show_train_labels)
    {
        ImGui::PopStyleColor();
    }
    ImGui::PopID();

    ImGui::PopStyleColor(); // Button
    ImGui::PopStyleColor(); // ButtonHovered
    ImGui::PopStyleColor(); // ButtonActive

    ImGui::End();

    // Виджеты HUD
    if (params->vehicles_handler != _train_profile_params.vehicles_handler)
    {
        _train_profile_params.vehicles_handler = params->vehicles_handler;
    }

    if (params->traffic_lights_handler != _train_profile_params.traffic_lights_handler)
    {
        _train_profile_params.traffic_lights_handler = params->traffic_lights_handler;
    }

    if (_train_profile_widget && params->hud_show_profile)
    {
        _train_profile_widget->show(bar_height, 300.0f - 10.0f, 20.0f, 20.0f);
    }

    if (params->hud_show_timetable)
    {
        showTimetable();
    }

    // Обновляем указатель на vehicles_handler при каждом кадре
    if (params->vehicles_handler != _trains_list_params.vehicles_handler)
    {
        _trains_list_params.vehicles_handler = params->vehicles_handler;
    }

    if (params->viewer_handler != _trains_list_params.viewer_handler)
    {
        _trains_list_params.viewer_handler = params->viewer_handler;
    }

    if (params->tcp_client != _trains_list_params.tcp_client)
    {
        _trains_list_params.tcp_client = params->tcp_client;
    }

    if (_trains_list_widget && params->hud_show_trains_list)
    {
        _trains_list_widget->show();
    }
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

    ImGui::PushStyleColor(ImGuiCol_WindowBg, params->hud_background);
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

        ImVec4 textColor = params->hud_timetable_future;

        if (timetable.stations[i].arr_delay || timetable.stations[i].dep_delay)
        {
            textColor = params->hud_timetable_delay;
        }
        else
        {
            if (i < timetable.curr_station_idx)
            {
                textColor = params->hud_timetable_past;
            }
            else
            {
                textColor = params->hud_timetable_future;
            }
        }

        if (i == timetable.curr_station_idx)
        {
            textColor = params->hud_timetable_current;
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
//
//------------------------------------------------------------------------------
void MyGui::showControlTooltip() const
{
    const ControlTooltip &tip = getControlTooltip();

    if (!tip.is_active)
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
        // ╨Ч╨░╨│╨╛╨╗╨╛╨▓╨╛╨║ ╨▓╤Л╤А╨░╨▓╨╜╨╕╨▓╨░╨╡╤В╤Б╤П ╨┐╨╛ ╤Ж╨╡╨╜╤В╤А╤Г ╨╛╨║╨╜╨░ ╨┐╨╛╨┤╤Б╨║╨░╨╖╨║╨╕
        const std::string title = tip.title.toStdString();
        const float title_w = ImGui::CalcTextSize(title.c_str()).x;
        const float avail_w = ImGui::GetContentRegionAvail().x;
        if (avail_w > title_w)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail_w - title_w) * 0.5f);
        }

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.4f, 1.0f));
        ImGui::TextUnformatted(title.c_str());
        ImGui::PopStyleColor();

        if (!tip.description.isEmpty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::TextUnformatted(tip.description.toStdString().c_str());
            ImGui::PopStyleColor();
        }

        if (!tip.usage.isEmpty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::TextUnformatted(tip.usage.toStdString().c_str());
            ImGui::PopStyleColor();
        }

        if (!tip.hot_keys.isEmpty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::TextUnformatted(tip.hot_keys.toStdString().c_str());
            ImGui::PopStyleColor();
        }

        if (!tip.state.isEmpty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::TextUnformatted(tip.state.toStdString().c_str());
            ImGui::PopStyleColor();
        }
    }

    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float MyGui::hudTopOffset() const
{
    // ╨Ю╤В╤Б╤В╤Г╨┐ ╨┐╨╛╤Б╤В╨╛╤П╨╜╨╜╤Л╨╣: ╤А╨╡╨╖╨╡╤А╨▓╨╕╤А╤Г╨╡╨╝ ╨╝╨╡╤Б╤В╨╛ ╨┐╨╛╨┤ ╨▒╨░╨╜╨╜╨╡╤А╤Л ╤Б╤В╨░╤В╤Г╤Б╨░ ╤Г╨┐╤А╨░╨▓╨╗╨╡╨╜╨╕╤П ╨╕
    // ╤Б╤В╨░╤В╨╕╤Б╤В╨╕╨║╤Г, ╨┤╨░╨╢╨╡ ╨╡╤Б╨╗╨╕ ╨╛╨╜╨╕ ╤Б╨╡╨╣╤З╨░╤Б ╨╜╨╡ ╨╛╤В╨╛╨▒╤А╨░╨╢╨░╤О╤В╤Б╤П
    float top = 0.0f;

    const char *text_no_controlled = "╨Э╨░╨╢╨╝╨╕╤В╨╡ Enter ╨┤╨╗╤П ╤Г╨┐╤А╨░╨▓╨╗╨╡╨╜╨╕╤П ╨┤╨░╨╜╨╜╨╛╨╣ ╨Я╨Х";
    const float h_no_controlled = ImGui::CalcTextSize(text_no_controlled).y + 20.0f;
    if (h_no_controlled > top)
        top = h_no_controlled;

    if (params->vehicles_handler)
    {
        VehicleExterior* cur = params->vehicles_handler->getCurrentVehicle();
        if (cur)
        {
            std::string msg = QString("╨Э╨░╨╢╨╝╨╕╤В╨╡ Enter ╨┤╨╗╤П ╤Г╨┐╤А╨░╨▓╨╗╨╡╨╜╨╕╤П ╨╕╨╖ ╨║╨░╨▒╨╕╨╜╤Л %1")
                                  .arg(cur->current_cabine_idx + 1).toStdString();
            const float h_cabine = ImGui::CalcTextSize(msg.c_str()).y + 20.0f;
            if (h_cabine > top)
                top = h_cabine;
        }
    }

    if (params->statistics_handler)
    {
        QString text = QString("Device: %1 ").arg(params->physicalDeviceName);
        text += QString("FPS:%1 (lowest:%2)")
                    .arg(params->statistics_handler->getAverageFPS(), 6, 'f', 1)
                    .arg(params->statistics_handler->getLowestFPS(), 6, 'f', 1);
        const float h_statistics = ImGui::CalcTextSize(text.toStdString().c_str()).y + 20.0f;
        if (h_statistics > top)
            top = h_statistics;
    }

    return (top > 0.0f) ? top + 2.0f : 8.0f;
}

//------------------------------------------------------------------------------
// Отправка сервисной команды текущего вагона на сервер.
// Используется существующий канал управления, протокол не меняется.
//------------------------------------------------------------------------------
void MyGui::sendCarServiceCommand(uint16_t action, float value) const
{
    if ((params->tcp_client == nullptr) || (params->vehicles_handler == nullptr))
    {
        return;
    }

    const int vehicle_idx = params->vehicles_handler->getCurrentVehicleIndex();

    if (vehicle_idx < 0)
    {
        return;
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << vehicle_idx;                      // индекс вагона
    stream << 0;                                // номер кабины
    stream << action;                           // ID сервисной команды
    stream << value;                            // 0/1 или величина
    stream << static_cast<quint32>(0);          // доп. параметров нет
    stream << static_cast<qint8>(0);            // приоритет

    params->tcp_client->slotSendVehicleControlCommand(data);
}

//------------------------------------------------------------------------------
// Меню вагона (Num3): сцепка и тележка, как в openRTS
//------------------------------------------------------------------------------
void MyGui::showCarMenu() const
{
    if ((params->vehicles_handler == nullptr) || (params->tcp_client == nullptr))
    {
        return;
    }

    const int count = params->vehicles_handler->getVehiclesCount();
    const int idx = params->vehicles_handler->getCurrentVehicleIndex();

    ImGui::SetNextWindowSize(ImVec2(440, 470), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin(tr(u8"Меню вагона", "Car menu").toUtf8().constData(),
                      &params->is_show_car_menu))
    {
        ImGui::End();
        return;
    }

    if ((idx < 0) || (idx >= count))
    {
        ImGui::TextUnformatted(tr(u8"Нет выбранного вагона",
                                  "No car selected").toUtf8().constData());
        ImGui::End();
        return;
    }

    // Текущее состояние вагона из диагностики сервера
    const simulator_diagnostics_update_t diagnostics =
            params->vehicles_handler->getDiagnostics();

    const simulator_vehicle_diagnostics_t* vd = nullptr;
    for (const auto& vehicle : diagnostics.vehicles)
    {
        if (vehicle.vehicle_idx == idx)
        {
            vd = &vehicle;
            break;
        }
    }

    // При смене вагона синхронизируем локальные отметки состояния
    if (params->car_menu_vehicle != idx)
    {
        params->car_menu_vehicle = idx;
        params->car_menu_handbrake = (vd != nullptr) && (vd->handbrake != 0);
        params->car_menu_end_valve_fwd = (vd != nullptr) && (vd->valve_fwd != 0);
        params->car_menu_end_valve_bwd = (vd != nullptr) && (vd->valve_bwd != 0);
        params->car_menu_chock_fwd = (vd != nullptr) && (vd->chock_fwd != 0);
        params->car_menu_chock_bwd = (vd != nullptr) && (vd->chock_bwd != 0);
    }

    ImGui::Text("%s %d %s %d",
                tr(u8"Вагон", "Car").toUtf8().constData(),
                idx + 1,
                tr(u8"из", "of").toUtf8().constData(),
                count);

    if (vd != nullptr)
    {
        ImGui::SameLine();
        ImGui::TextDisabled("(%.1f %s)", vd->speed_kmh,
                            tr(u8"км/ч", "km/h").toUtf8().constData());
    }

    ImGui::Separator();

    if (ImGui::BeginTabBar("##car_menu_tabs"))
    {
        if (ImGui::BeginTabItem(tr(u8"Сцепка", "Coupling").toUtf8().constData()))
        {
            params->car_menu_page = 0;

            const bool moving = (vd != nullptr)
                    && (std::abs(vd->speed_kmh) > 1.8f);

            // Один конец вагона: расцепка, концевой кран и тормозной рукав
            auto draw_end = [&](bool fwd)
            {
                const char* end_name = fwd
                        ? tr(u8"Передний конец", "Front end").toUtf8().constData()
                        : tr(u8"Задний конец", "Rear end").toUtf8().constData();

                ImGui::TextUnformatted(end_name);
                ImGui::Separator();

                const bool coupled = (vd == nullptr)
                        || (fwd ? (vd->coupled_fwd != 0) : (vd->coupled_bwd != 0));

                ImGui::BeginDisabled(!coupled || moving);
                if (ImGui::Button(fwd
                                  ? tr(u8"Расцепить спереди", "Uncouple front").toUtf8().constData()
                                  : tr(u8"Расцепить сзади", "Uncouple rear").toUtf8().constData(),
                                  ImVec2(-1.0f, 0.0f)))
                {
                    sendCarServiceCommand(fwd ? vehicle_service::UncoupleFwd
                                              : vehicle_service::UncoupleBwd, 1.0f);
                }
                ImGui::EndDisabled();

                if (!coupled)
                {
                    ImGui::TextDisabled(tr(u8"Свободный конец",
                                           "Free end").toUtf8().constData());
                }
                else if (moving)
                {
                    ImGui::TextDisabled(tr(u8"Расцепка только на остановке",
                                           "Uncouple only at standstill").toUtf8().constData());
                }

                bool& valve = fwd ? params->car_menu_end_valve_fwd
                                  : params->car_menu_end_valve_bwd;
                if (ImGui::Checkbox(fwd
                                    ? tr(u8"Передний кран открыт", "Front valve open").toUtf8().constData()
                                    : tr(u8"Задний кран открыт", "Rear valve open").toUtf8().constData(),
                                    &valve))
                {
                    sendCarServiceCommand(fwd ? vehicle_service::EndValveFwd
                                              : vehicle_service::EndValveBwd,
                                          valve ? 1.0f : 0.0f);
                }

                const int hose = (vd == nullptr)
                        ? 1
                        : (fwd ? vd->hose_fwd : vd->hose_bwd);

                QByteArray hose_text = tr(u8"на крюке", "on holder").toUtf8();
                if (hose == 0)
                {
                    hose_text = tr(u8"соединён", "connected").toUtf8();
                }
                else if (hose == 2)
                {
                    hose_text = tr(u8"оборван", "torn").toUtf8();
                }

                ImGui::Text("%s: %s",
                            tr(u8"Тормозной рукав", "Brake hose").toUtf8().constData(),
                            hose_text.constData());

                if (hose != 0)
                {
                    ImGui::BeginDisabled(hose == 2);
                    if (ImGui::Button(fwd
                                      ? tr(u8"Соединить рукав спереди", "Connect front hose").toUtf8().constData()
                                      : tr(u8"Соединить рукав сзади", "Connect rear hose").toUtf8().constData(),
                                      ImVec2(-1.0f, 0.0f)))
                    {
                        sendCarServiceCommand(fwd ? vehicle_service::HoseConnectFwd
                                                  : vehicle_service::HoseConnectBwd, 1.0f);
                    }
                    ImGui::EndDisabled();
                }
                else
                {
                    if (ImGui::Button(fwd
                                      ? tr(u8"Повесить рукав спереди", "Hang front hose").toUtf8().constData()
                                      : tr(u8"Повесить рукав сзади", "Hang rear hose").toUtf8().constData(),
                                      ImVec2(-1.0f, 0.0f)))
                    {
                        sendCarServiceCommand(fwd ? vehicle_service::HoseHangFwd
                                                  : vehicle_service::HoseHangBwd, 1.0f);
                    }
                }

                ImGui::Spacing();
            };

            draw_end(true);
            draw_end(false);

            ImGui::Separator();
            ImGui::TextWrapped(tr(u8"Сцепка выполняется автоматически при сближении составов до 10 м.",
                                  "Coupling is automatic when trains come within 10 m.").toUtf8().constData());

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(tr(u8"Тележка", "Bogie").toUtf8().constData()))
        {
            params->car_menu_page = 1;

            // Ручной (стояночный) тормоз
            if (ImGui::Checkbox(tr(u8"Ручной тормоз", "Handbrake").toUtf8().constData(),
                                &params->car_menu_handbrake))
            {
                sendCarServiceCommand(vehicle_service::ParkingBrake,
                                      params->car_menu_handbrake ? 1.0f : 0.0f);
            }

            ImGui::Separator();
            ImGui::TextUnformatted(tr(u8"Тормозные башмаки",
                                      "Wheel chocks").toUtf8().constData());

            if (ImGui::Checkbox(tr(u8"Передняя тележка", "Front bogie").toUtf8().constData(),
                                &params->car_menu_chock_fwd))
            {
                sendCarServiceCommand(vehicle_service::ChockFwd,
                                      params->car_menu_chock_fwd ? 1.0f : 0.0f);
            }

            if (ImGui::Checkbox(tr(u8"Задняя тележка", "Rear bogie").toUtf8().constData(),
                                &params->car_menu_chock_bwd))
            {
                sendCarServiceCommand(vehicle_service::ChockBwd,
                                      params->car_menu_chock_bwd ? 1.0f : 0.0f);
            }

            ImGui::Separator();
            ImGui::TextUnformatted(tr(u8"Груз", "Cargo").toUtf8().constData());

            if ((vd != nullptr) && (vd->cargo_configured != 0))
            {
                ImGui::Text("%s: %s",
                            tr(u8"Тип", "Type").toUtf8().constData(),
                            vd->cargo_type.isEmpty()
                            ? tr(u8"нет", "none").toUtf8().constData()
                            : vd->cargo_type.toUtf8().constData());
                ImGui::Text("%s: %.1f %s",
                            tr(u8"Масса", "Mass").toUtf8().constData(),
                            vd->cargo_mass_t,
                            tr(u8"т", "t").toUtf8().constData());
                ImGui::ProgressBar(vd->cargo_fill, ImVec2(-1.0f, 0.0f));
            }
            else
            {
                ImGui::TextDisabled(tr(u8"Секция [CargoWagon] не настроена",
                                       "[CargoWagon] is not configured").toUtf8().constData());
            }

            ImGui::Separator();
            ImGui::TextUnformatted(tr(u8"Пассажиры", "Passengers").toUtf8().constData());

            if ((vd != nullptr) && (vd->passengers_configured != 0))
            {
                ImGui::Text("%d / %d",
                            vd->passenger_count,
                            vd->passenger_capacity);
            }
            else
            {
                ImGui::TextDisabled(tr(u8"Секция [PassengerCar] не настроена",
                                       "[PassengerCar] is not configured").toUtf8().constData());
            }

            ImGui::Separator();
            ImGui::TextDisabled(tr(u8"ЭПТ, СМЕ, ВВ-шина, отпускной и стоп-кран в RRS пока не моделируются.",
                                   "EPB, MU, HV bus, release and stop valves are not simulated in RRS yet.").toUtf8().constData());

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
