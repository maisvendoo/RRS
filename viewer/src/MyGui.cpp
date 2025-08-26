#include "MyGui.h"

#include "filesystem.h"
#include "datetime.h"

#include "Skybox.h"
#include "UpdateStatisticsHandler.h"
#include "UpdateControlToServerHandler.h"
#include "VehiclesHandler.h"
#include "spa.h"

#include <vsg/io/Options.h>
#include <vsg/maths/common.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/transform.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/app/Viewer.h>
#include <vsgImGui/imgui.h>

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

    if (params->vehicles_handler)
    {
        params->sim_time = params->vehicles_handler->getDateTime();
        if (params->sim_time && params->use_server_time)
        {
            params->year = params->sim_time->date.year();
            params->month = params->sim_time->date.month();
            params->day = params->sim_time->date.day();
            params->hour = params->sim_time->time.hour();
            params->minute = params->sim_time->time.minute();
            params->sec = params->sim_time->time.sec();
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
        params->is_no_controlled =
                        (params->vehicles_handler->getCurrentVehicleIndex() !=
                        params->vehicles_handler->getControlledVehicleIndex());

        VehicleExterior* cur_vehicle = params->vehicles_handler->getCurrentVehicle();
        params->is_no_cabine_control = ((cur_vehicle != nullptr) &&
                                        (cur_vehicle->controlled_cabine_idx != cur_vehicle->current_cabine_idx));
    }
    else
    {
        params->prev_F9 = false;
        params->is_show_debug_msg = false;
        params->is_no_controlled = false;

        check_date_time();
    }

    simulator_time_t datetime({static_cast<int16_t>(params->year), static_cast<uint8_t>(params->month), static_cast<uint8_t>(params->day)},
                              {static_cast<uint8_t>(params->hour), static_cast<uint8_t>(params->minute), static_cast<uint8_t>(params->sec)});
    params->skybox->setDateTime(datetime);
    params->year = datetime.date.year();
    params->month = datetime.date.month();
    params->day = datetime.date.day();
    params->hour = datetime.time.hour();
    params->minute = datetime.time.minute();
    params->sec = datetime.time.sec();

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
    QString text = QString("FPS:%1 (lowest:%2)")
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

    ImGui::RadioButton("Использовать локальное время:", &(params->use_server_time), 0);
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
    if (ImGui::SliderInt("Day seconds", &day_seconds, 0, 86400))
    {
        params->hour = day_seconds / 3600;
        params->minute = (day_seconds - params->hour * 3600) / 60;
        params->sec = day_seconds % 60;
    }

    // Calculate sun position from latitude, longitude, date and time
    //--------------------------------------------------------------------------
    // Координаты РГУПСа (для теста)
    static constexpr double latitude = 47.2504559;
    static constexpr double longitude = 39.6982501;

    spa_data spa;
    spa.year = params->year;
    spa.month = params->month;
    spa.day = params->day;
    spa.hour = params->hour - 1;
    spa.minute = params->minute;
    spa.second = params->sec;
    spa.delta_ut1 = 0.0;
    spa.delta_t = 0.0;
    spa.timezone = 3.0;
    spa.longitude = longitude;
    spa.latitude = latitude;
    spa.elevation = 0.0;
    spa.pressure = 0.0;
    spa.temperature = 0.0;
    spa.slope = 0.0;
    spa.azm_rotation = 0.0;
    spa.atmos_refract = 0.5667;
    spa.function = SPA_ALL;

    spa_calculate(&spa);

    ImGui::Text("latitude: %f", latitude);
    ImGui::Text("longitude: %f", longitude);
    ImGui::Text("azimuth: %f", spa.azimuth);
    ImGui::Text("altitude: %f", spa.e);
    //--------------------------------------------------------------------------

    ImGui::ColorEdit3("Ambient color", params->ambient_color);
    ImGui::SliderFloat("Ambient intensity", params->ambient_intensity, 0.0f, 1.0f);
    ImGui::ColorEdit3("Sun color", params->sun_color);
    ImGui::SliderFloat("Sun intensity", params->sun_intensity, 0.0f, 5.0f);

    // if (params->skybox_textures.size() > 0)
    // {
    //     ImGui::SliderInt("Skybox texture", &params->skybox_texture_index, 1, params->skybox_textures.size());
    // }

    ImGui::End();

    const double azimuth_rad = vsg::radians(spa.azimuth);
    const double altitude_rad = vsg::radians(spa.e);

    vsg::dvec3 sun_dir;
    sun_dir.x = -std::cos(altitude_rad) * std::sin(azimuth_rad);
    sun_dir.y = -std::cos(altitude_rad) * std::cos(azimuth_rad);
    sun_dir.z = -std::sin(altitude_rad);
    sun_dir = vsg::normalize(sun_dir);
    *params->sun_direction_d = sun_dir;

    if (params->prev_skybox_texture_index == params->skybox_texture_index)
    {
        return;
    }

    // params->prev_skybox_texture_index = params->skybox_texture_index;
    // if ((params->skybox_textures.size() > 0)
    //     && (params->skybox_texture_index > 0)
    //     && (params->skybox_texture_index <= params->skybox_textures.size()))
    // {
    //     vsg::ref_ptr<vsg::ubvec4Array2D> selected_data = params->skybox_textures[params->skybox_texture_index - 1];
    //     auto selected_data_pixel = selected_data->begin();
    //     auto texture_pixel = params->skybox_texture_data->begin();
    //     while (texture_pixel != params->skybox_texture_data->end())
    //     {
    //         *texture_pixel = *selected_data_pixel;
    //         ++selected_data_pixel;
    //         ++texture_pixel;
    //     }
    //     params->skybox_texture_data->dirty();
    // }
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
    std::string msg = QString("Нажмите Enter для управления из кабины %1")
                            .arg(params->vehicles_handler->getCurrentVehicle()->current_cabine_idx + 1)
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
void MyGui::check_date_time() const
{
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
