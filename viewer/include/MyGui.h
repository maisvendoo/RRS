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
    bool prev_F9 = false;
    bool is_show_debug_msg = false;
    bool is_no_controlled = false;
    QString status = "";
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

    void showDebugMsg() const;

    void showNoControlled() const;

    void printObject(const vsg::ref_ptr<vsg::Object>& object) const;
};

#endif // MY_GUI_H
