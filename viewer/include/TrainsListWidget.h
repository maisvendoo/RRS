#ifndef     TRAINS_LIST_WIDGET
#define     TRAINS_LIST_WIDGET

#include    <VehiclesHandler.h>
#include    <vsgImGui/imgui.h>
#include    <QString>
#include    <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TrainsListWidgetParams
{
    VehiclesHandler *vehicles_handler = nullptr;
    bool is_visible = true;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrainsListWidget
{
public:

    TrainsListWidget(TrainsListWidgetParams *params);

    ~TrainsListWidget() = default;

    void show();

private:

    TrainsListWidgetParams* _params;

    std::vector<int> _cached_trains_ids;

    int _selected_train_id = -1;

    bool _initialized = false;

    void updateCachedTrainsList();

    void renderTrainsList();

    void selectTrain(int first_vehicle_id);
};

#endif
