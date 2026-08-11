#include    <TrainsListWidget.h>
#include    <Logger.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainsListWidget::TrainsListWidget(TrainsListWidgetParams *params)
    : _params(params)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainsListWidget::show()
{
    if (!_params || !_params->vehicles_handler || !_params->is_visible)
    {
        return;
    }

    updateCachedTrainsList();
    renderTrainsList();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainsListWidget::updateCachedTrainsList()
{
    auto *handler = _params->vehicles_handler;

    if (!handler)
    {
        return;
    }

    const auto &trains = handler->getTrainsInfo();
    _cached_trains_ids.clear();

    if (trains.empty())
    {
        return;
    }

    for (const auto &train : trains)
    {
        _cached_trains_ids.push_back(train.first_vehicle_id);
    }

    if (_selected_train_id >= 0)
    {
        bool found = false;

        for (int id : _cached_trains_ids)
        {
            if (id == _selected_train_id)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            _selected_train_id = -1;
        }
    }

    _initialized = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainsListWidget::renderTrainsList()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainsListWidget::selectTrain(int first_vehicle_id)
{
    auto *handler = _params->vehicles_handler;

    if (!handler)
    {
        return;
    }

    const int current_vehicle_id = handler->getCurrentVehicleIndex();

    if (current_vehicle_id != first_vehicle_id)
    {
        handler->setCurrentVehicle(first_vehicle_id);
        LOG_INFO("Swithced to train with first vehicle %d", first_vehicle_id);
    }
}
