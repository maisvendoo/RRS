#ifndef MY_GUI_H
#define MY_GUI_H

#include <vsg/commands/Command.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/observer_ptr.h>
#include <QString>

#include <TrainsListWidget.h>
#include <TrainProfileHintWidget.h>

struct simulator_time_t;
class NewSkybox;
class Skybox;
class Sun;
class VehiclesHandler;
class UpdateViewerHandler;
class UpdateStatisticsHandler;
class UpdateControlToServerHandler;
class TcpClient;
class TrafficLightsHandler;
class StationsHandler;

struct GUIParams final : public vsg::Inherit<vsg::Object, GUIParams>
{
    GUIParams() {}

    vsg::observer_ptr<vsg::Viewer> viewer;
    simulator_time_t* sim_time = nullptr;
    int speed_factor = 1;

    // Skybox *skybox = nullptr;
    NewSkybox* new_skybox = nullptr;  // Owned by RouteViewer
    VehiclesHandler *vehicles_handler = nullptr;
    UpdateViewerHandler *viewer_handler = nullptr;
    UpdateStatisticsHandler *statistics_handler = nullptr;
    UpdateControlToServerHandler *controls_handler = nullptr;
    TrafficLightsHandler *traffic_lights_handler = nullptr;
    StationsHandler *stations_handler = nullptr;
    TcpClient *tcp_client = nullptr;

    vsg::ref_ptr<Sun> sun;

    double latitude = 47.2;
    double longitude = 39.7;

    int use_server_time = true;
    int16_t year = 2000;
    int16_t month = 1;
    int16_t day = 1;
    int16_t hour = 0;
    int16_t minute = 0;
    int16_t sec = 0;
    int16_t msec = 0;

    bool prev_Esc = false;
    bool is_show_quit_dialog = false;

    bool prev_F11 = false;
    bool is_show_statistics = false;

    bool prev_F10 = false;
    bool is_show_settings = false;

    bool prev_F9 = false;
    bool is_show_debug_msg = false;

    bool prev_F8 = false;
    bool is_show_trane_rename_dialog = false;

    bool prev_F7 = false;
    bool is_show_HUD = false;

    /// Флаги видимости виджетов HUD
    bool hud_show_profile = true;
    bool hud_show_timetable = true;
    bool hud_show_trains_list = true;
    bool hud_show_stations = true;

    bool is_no_controlled = false;

    bool is_no_cabine_control = false;

    /// Дальность профиля пути назад/вперёд от середины поезда, м (запрос к серверу)
    double train_profile_backward = 4000.0;
    double train_profile_forward = 4000.0;

    QString status = "";
    QString physicalDeviceName = "";
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MyGui final : public vsg::Inherit<vsg::Command, MyGui>
{
public:
    MyGui(vsg::ref_ptr<GUIParams> in_params, vsg::ref_ptr<vsg::Options> options = {});

    void compile(vsg::Context& context) override;

    void record(vsg::CommandBuffer& cb) const override;

private:
    vsg::ref_ptr<GUIParams> params;

    mutable TrainsListWidget *_trains_list_widget = nullptr;

    mutable TrainsListWidgetParams _trains_list_params;

    mutable TrainProfileHintWidget *_train_profile_widget = nullptr;

    mutable TrainProfileHintWidgetParams _train_profile_params;

    float font_size = 20.0f;    

    void showStatus() const;

    void showQuitDialog() const;

    void showStatistics() const;

    void showSettings() const;

    void showDebugMsg() const;

    void showNoControlled() const;

    void showNoCabineControl() const;

    void showTrainRenameDialog() const;

    void showPauseState() const;

    void showHUD() const;

    void showTimetable() const;

    float hudTopOffset() const;

    void printObject(const vsg::ref_ptr<vsg::Object>& object) const;

    void check_date_time() const;
};

#endif // MY_GUI_H
