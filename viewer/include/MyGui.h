#pragma once
#ifndef MY_GUI_H
#define MY_GUI_H

#include <vsg/commands/Command.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/observer_ptr.h>
#include <QString>

class VehiclesHandler;
class UpdateStatisticsHandler;

struct GUIParams final : public vsg::Inherit<vsg::Object, GUIParams>
{
    GUIParams() {}

    vsg::observer_ptr<vsg::Viewer> viewer;
    VehiclesHandler *vehicles_handler = nullptr;
    UpdateStatisticsHandler *statistics_handler = nullptr;

    bool prev_Esc = false;
    bool is_show_quit_dialog = false;

    bool prev_F11 = false;
    bool is_show_statistics = false;

    bool prev_F10 = false;
    bool is_show_settings = false;

    bool prev_F9 = false;
    bool is_show_debug_msg = false;

    bool is_no_controlled = false;

    QString status = "";

    float* ambient_color = nullptr;
    float* ambient_intensity = nullptr;
    float* sun_color = nullptr;
    vsg::dvec3* sun_direction_d = nullptr;
    float* sun_intensity = nullptr;

    float sun_azimuth_degrees = 45.0f;
    float sun_altitude_degrees = 45.0f;

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

    void printObject(const vsg::ref_ptr<vsg::Object>& object) const;
};

#endif // MY_GUI_H
