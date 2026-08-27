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