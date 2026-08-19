#include "settings/SceneSettings.h"

#include <CfgReader.h>

#include <QString>

scene_settings_t::scene_settings_t()
    : num_lights(200)
{
}

void scene_settings_t::read(CfgReader& cfg)
{
    const QString section = "Scene";

    cfg.getInt(section, "NumLights", num_lights);
}
