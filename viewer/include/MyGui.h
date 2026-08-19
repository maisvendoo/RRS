#ifndef MY_GUI_H
#define MY_GUI_H

#include <vsg/commands/Command.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/observer_ptr.h>
#include <QString>

struct simulator_time_t;
class NewSkybox;
class RouteViewer;
class Skybox;
class Sun;
class VehiclesHandler;
class UpdateStatisticsHandler;
class UpdateControlToServerHandler;
class TcpClient;

struct GUIParams final : public vsg::Inherit<vsg::Object, GUIParams>
{
    GUIParams() {}

    vsg::observer_ptr<vsg::Viewer> viewer;
    simulator_time_t* sim_time = nullptr;
    int speed_factor = 1;

    // Skybox *skybox = nullptr;
    NewSkybox* new_skybox = nullptr;  // Owned by RouteViewer
    VehiclesHandler *vehicles_handler = nullptr;
    UpdateStatisticsHandler *statistics_handler = nullptr;
    UpdateControlToServerHandler *controls_handler = nullptr;
    TcpClient *tcp_client = nullptr;

    // Владелец — применяется пресет графики из GUI (ТЗ "Графика")
    RouteViewer* route_viewer = nullptr;
    int graphics_preset_index = 1;        ///< Текущий пресет: 0-Legacy...4-Custom
    bool graphics_needs_restart = false;  ///< Полное применение после перезапуска

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

    /// Окно диагностики составов (ТЗ "Промт статистики вагонов"):
    /// F3 - вкл/выкл, F4 - свёрнутый/полный режим
    bool prev_F3 = false;
    bool is_show_diagnostics = false;

    bool prev_F4 = false;
    bool diagnostics_full_mode = false;

    bool is_no_controlled = false;

    bool is_no_cabine_control = false;

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

    float font_size = 20.0f;    

    void showStatus() const;

    void showQuitDialog() const;

    void showStatistics() const;

    void showSettings() const;

    void showGraphicsSettings() const;

    void showDebugMsg() const;

    /// Диагностика составов (ТЗ "Промт статистики вагонов", F3/F4)
    void showDiagnostics() const;

    void showNoControlled() const;

    void showNoCabineControl() const;

    void showTrainRenameDialog() const;

    void showPauseState() const;

    void showHUD() const;

    void showTimetable() const;

    void printObject(const vsg::ref_ptr<vsg::Object>& object) const;

    void check_date_time() const;
};

#endif // MY_GUI_H
