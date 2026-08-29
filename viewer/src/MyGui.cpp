#include "MyGui.h"

#include "filesystem.h"
#include "datetime.h"

#include "NewSkybox.h"
// #include "Skybox.h"
#include "Sun.h"
#include "UpdateStatisticsHandler.h"
#include "UpdateControlToServerHandler.h"
#include "VehiclesHandler.h"
#include "UpdateViewerHandler.h"
#include "StationsHandler.h"
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
    _train_profile_params.hud_train_profile_mast = params->hud_train_profile_mast;
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

        // При глобальном скрытии HUD прячем подписи станций вне зависимости
        // от состояния кнопки; при показе возвращаем согласно кнопке
        if (params->stations_handler)
        {
            params->stations_handler->setVisible(
                params->is_show_HUD && params->hud_show_stations);
        }
    }
    params->prev_F7 = ImGui::IsKeyPressed(ImGuiKey_F7);

    if (params->is_show_HUD)
    {
        showHUD();
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
            ImGui::SliderFloat("Sun intensity", &(params->sun->sun->intensity), 0.0f, 10.0f, "%.3f");
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

    ImGui::End();
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
//
//------------------------------------------------------------------------------
void MyGui::showNoControlled() const
{
    const char *text = "Нажмите Enter для управления данной ПЕ";
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

    const float total_w = 480.0f;

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
    float btn_w = (total_w - pad * 2 - gap * 3) / 4.0f;
    float btn_h = ImGui::GetWindowHeight() - 4.0f;

    ImGui::SetCursorPos(ImVec2(pad, 2.0f));
    ImGui::PushID("hud_profile");
    if (params->hud_show_profile)
        ImGui::PushStyleColor(ImGuiCol_Button, params->hud_button_on);
    if (ImGui::Button(u8"Профиль", ImVec2(btn_w, btn_h)))
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
    if (ImGui::Button(u8"График", ImVec2(btn_w, btn_h)) && has_timetable)
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
    if (ImGui::Button(u8"Поезда", ImVec2(btn_w, btn_h)))
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
    if (ImGui::Button(u8"Станции", ImVec2(btn_w, btn_h)) && has_stations)
    {
        params->hud_show_stations = !params->hud_show_stations;
        params->stations_handler->setVisible(params->hud_show_stations);
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
float MyGui::hudTopOffset() const
{
    // Отступ постоянный: резервируем место под баннеры статуса управления и
    // статистику, даже если они сейчас не отображаются
    float top = 0.0f;

    const char *text_no_controlled = "Нажмите Enter для управления данной ПЕ";
    const float h_no_controlled = ImGui::CalcTextSize(text_no_controlled).y + 20.0f;
    if (h_no_controlled > top)
        top = h_no_controlled;

    if (params->vehicles_handler)
    {
        VehicleExterior* cur = params->vehicles_handler->getCurrentVehicle();
        if (cur)
        {
            std::string msg = QString("Нажмите Enter для управления из кабины %1")
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
