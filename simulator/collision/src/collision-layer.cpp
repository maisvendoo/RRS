//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      Collision layer utilities and body groups
//
//------------------------------------------------------------------------------

#include    "collision-layer.h"

#include    <cstring>

namespace collision
{

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const char* layerName(Layer layer)
{
    switch (layer)
    {
    case Layer::Terrain:         return "terrain";
    case Layer::Rail:            return "rail";
    case Layer::Track:           return "track";
    case Layer::Infrastructure:  return "infrastructure";
    case Layer::ContactNetwork:  return "contact_network";
    case Layer::Decoration:      return "decoration";
    case Layer::Vegetation:      return "vegetation";
    case Layer::Train:           return "train";
    case Layer::Bogie:           return "bogie";
    case Layer::Wheel:           return "wheel";
    case Layer::RoadVehicle:     return "road_vehicle";
    case Layer::Player:          return "player";
    default:                     return "unknown";
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Layer layerFromString(const char* name, bool* ok, Layer fallback)
{
    if (name != nullptr)
    {
        for (std::uint16_t i = 0; i < layer_count; ++i)
        {
            Layer layer = static_cast<Layer>(i);
            if (std::strcmp(name, layerName(layer)) == 0)
            {
                if (ok != nullptr)
                    *ok = true;
                return layer;
            }
        }
    }

    if (ok != nullptr)
        *ok = false;
    return fallback;
}

} // namespace collision
