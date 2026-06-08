#include "editor/settings/SceneSettings.h"

#include <CfgReader.h>
#include <Journal.h>

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

void scene_settings_t::print_in_journal() const
{
    Journal* const journal = Journal::instance();

    journal->debug("Scene settings:");
    journal->debug("    num_lights: " + QString::number(num_lights));
}
