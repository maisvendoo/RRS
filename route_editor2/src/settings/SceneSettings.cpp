#include "editor/settings/SceneSettings.h"

#include <CfgReader.h>
#include <Journal.h>

#include <QString>

scene_settings_t::scene_settings_t()
    : num_lights(200)
    , max_object_count(1000000)
{
}

void scene_settings_t::read(CfgReader& cfg)
{
    const QString section = "Scene";

    cfg.getInt(section, "NumLights", num_lights);
    cfg.getInt(section, "MaxObjectCount", max_object_count);
}

void scene_settings_t::print_in_journal() const
{
    Journal* const journal = Journal::instance();

    journal->debug("Scene settings:");
    journal->debug("    num_lights: " + QString::number(num_lights));
    journal->debug("    max_object_count: " + QString::number(max_object_count));
}
