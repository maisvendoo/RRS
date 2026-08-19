#ifndef TRAIN_PROFILE_HINT_WIDGET_H
#define TRAIN_PROFILE_HINT_WIDGET_H

#include <VehiclesHandler.h>
#include <vsgImGui/imgui.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TrainProfileHintWidgetParams
{
    VehiclesHandler *vehicles_handler = nullptr;
    bool is_visible = true;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrainProfileHintWidget
{
public:

    TrainProfileHintWidget(TrainProfileHintWidgetParams *params);

    ~TrainProfileHintWidget() = default;

    void show(float top_y, float bottom_y);

private:

    void drawProfile() const;

    TrainProfileHintWidgetParams *_params;

    simulator_train_profile_update_t _profile;

    bool _profile_valid = false;
};

#endif // TRAIN_PROFILE_HINT_WIDGET_H