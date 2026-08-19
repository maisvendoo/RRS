#ifndef TRAIN_PROFILE_HINT_WIDGET_H
#define TRAIN_PROFILE_HINT_WIDGET_H

#include <vsgImGui/imgui.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TrainProfileHintWidgetParams
{
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

    TrainProfileHintWidgetParams *_params;
};

#endif // TRAIN_PROFILE_HINT_WIDGET_H