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
class Skybox;
class Sun;
class VehiclesHandler;
class UpdateStatisticsHandler;
class UpdateControlToServerHandler;

struct GUIParams final : public vsg::Inherit<vsg::Object, GUIParams>
{
    GUIParams() {}

    vsg::observer_ptr<vsg::Viewer> viewer;
    simulator_time_t* sim_time = nullptr;

    // TODO: Нигде не используется
    simulator_time_t* local_time = nullptr;

    // Skybox *skybox = nullptr;
    NewSkybox* new_skybox = nullptr;
    VehiclesHandler *vehicles_handler = nullptr;
    UpdateStatisticsHandler *statistics_handler = nullptr;
    UpdateControlToServerHandler *controls_handler = nullptr;

    int use_server_time = true;
    bool was_server_time_unavailable = true;
    int16_t year = 2000;
    int16_t month = 1;
    int16_t day = 1;
    int16_t hour = 0;
    int16_t minute = 0;
    int16_t sec = 0;

    bool prev_Esc = false;
    bool is_show_quit_dialog = false;

    bool prev_F11 = false;
    bool is_show_statistics = false;

    bool prev_F10 = false;
    bool is_show_settings = false;

    bool prev_F9 = false;
    bool is_show_debug_msg = false;

    bool is_no_controlled = false;

    bool is_no_cabine_control = false;

    QString status = "";

    vsg::ref_ptr<Sun> sun;

    int skybox_texture_index = 1;
    int prev_skybox_texture_index = 1;
    vsg::ref_ptr<vsg::ubvec4Array2D> skybox_texture_data;
    std::vector<vsg::ref_ptr<vsg::ubvec4Array2D>> skybox_textures;
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

    void showDebugMsg() const;

    void showNoControlled() const;

    void showNoCabineControl() const;

    void printObject(const vsg::ref_ptr<vsg::Object>& object) const;

    void check_date_time() const;
};

#endif // MY_GUI_H
