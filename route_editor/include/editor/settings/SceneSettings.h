#ifndef EDITOR_SETTINGS_SCENE_SETTINGS_H
#define EDITOR_SETTINGS_SCENE_SETTINGS_H

class CfgReader;

struct scene_settings_t
{
    int num_lights;
    double culling_tiles_size_0 = 4000.0;
    double culling_tiles_size_1 = 32000.0;

    scene_settings_t();
    void read(CfgReader& cfg);
};

#endif // EDITOR_SETTINGS_SCENE_SETTINGS_H
