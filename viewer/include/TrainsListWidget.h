#ifndef     TRAINS_LIST_WIDGET
#define     TRAINS_LIST_WIDGET

#include    <VehiclesHandler.h>
#include    <UpdateViewerHandler.h>
#include    <vsgImGui/imgui.h>
#include    <QString>
#include    <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TrainsListWidgetParams
{
    VehiclesHandler *vehicles_handler = nullptr;
    UpdateViewerHandler* viewer_handler = nullptr;
    bool is_visible = true;

    /// Цвета виджетов интерфейса (HUD), RGBA в диапазоне 0.0 - 1.0
    ImVec4 hud_background = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
    ImVec4 hud_text = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 hud_current_train = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    ImVec4 hud_controlled_train = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
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

    void syncSelectionWithCurrentTrain();

    void renderTrainsList();

    void selectTrain(int first_vehicle_id);
};

#endif
