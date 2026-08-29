#ifndef TRAIN_PROFILE_HINT_WIDGET_H
#define TRAIN_PROFILE_HINT_WIDGET_H

#include <VehiclesHandler.h>
#include <vsgImGui/imgui.h>

class TrafficLightsHandler;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TrainProfileHintWidgetParams
{
    VehiclesHandler *vehicles_handler = nullptr;
    TrafficLightsHandler *traffic_lights_handler = nullptr;
    bool is_visible = true;
    /// Запрошенные дальности профиля назад/вперёд от середины поезда, м
    float backward_m = 4000.0f;
    float forward_m = 4000.0f;
    /// Цвет фона виджета (RGBA, 0.0 - 1.0)
    ImVec4 hud_background = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
    /// Цвета виджета профиля пути (RGBA, 0.0 - 1.0)
    ImVec4 hud_train_profile_grid = ImVec4(0.353f, 0.353f, 0.353f, 0.588f);
    ImVec4 hud_train_profile_grid_label = ImVec4(0.745f, 0.745f, 0.745f, 0.863f);
    ImVec4 hud_train_profile_baseline = ImVec4(0.502f, 0.502f, 0.502f, 1.0f);
    ImVec4 hud_train_profile_curve = ImVec4(0.0f, 0.4f, 0.8f, 1.0f);
    ImVec4 hud_train_profile_uncontrolled = ImVec4(0.251f, 0.502f, 0.0f, 1.0f);
    ImVec4 hud_train_profile_current = ImVec4(0.753f, 0.753f, 0.0f, 1.0f);
    ImVec4 hud_train_profile_controlled = ImVec4(0.753f, 0.251f, 0.251f, 1.0f);
    ImVec4 hud_train_profile_station_text = ImVec4(0.0f, 0.784f, 1.0f, 1.0f);
    ImVec4 hud_train_profile_mast = ImVec4(0.863f, 0.863f, 0.863f, 1.0f);
    ImVec4 hud_train_profile_signal_letter = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 hud_train_profile_speed_limit_border = ImVec4(0.353f, 0.353f, 0.353f, 0.588f);
    ImVec4 hud_train_profile_speed_limit_fill = ImVec4(0.353f, 0.353f, 0.353f, 0.157f);
    ImVec4 hud_train_profile_speed_limit_text = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    ImVec4 hud_train_profile_speed_limit_bg = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrainProfileHintWidget
{
public:

    TrainProfileHintWidget(TrainProfileHintWidgetParams *params);

    ~TrainProfileHintWidget() = default;

    void show(float top_y, float bottom_y, float left_inset = 0.0f, float right_inset = 0.0f);

private:

    struct PlotTransform
    {
        float cx = 0.0f;
        float cy = 0.0f;
        float x0 = 0.0f;
        float x1 = 0.0f;
        float y0 = 0.0f;
        float y1 = 0.0f;
        float origin_elev = 0.0f;
        float rel_min = 0.0f;
        float rel_max = 0.0f;
        float x_scale = 1.0f;
        float y_scale = 1.0f;

        float map_x(float d) const { return cx + d * x_scale; }
        float map_y(float rel) const { return cy - rel * y_scale; }
    };

    void drawProfile() const;

    void drawTrain(const PlotTransform& plot) const;

    void drawTrainNames(const PlotTransform& plot) const;

    void drawSignals(const PlotTransform& plot) const;

    void drawStations(const PlotTransform& plot) const;

    void drawSpeedLimits(const PlotTransform& plot) const;

    TrainProfileHintWidgetParams *_params;

    simulator_train_profile_update_t _profile;

    bool _profile_valid = false;
};

#endif // TRAIN_PROFILE_HINT_WIDGET_H