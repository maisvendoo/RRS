#include "TrainExteriorHandler.h"

TrainExteriorHandler::TrainExteriorHandler(const settings_t& settings, const std::unique_ptr<SoundManager>& sm)
    : settings_delay()
{
    settings_delay = (settings.vehicle_controled_update_interval + settings.client_delay) * 0.001;
}
