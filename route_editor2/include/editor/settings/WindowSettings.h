#ifndef EDITOR_WINDOW_SETTINGS_H
#define EDITOR_WINDOW_SETTINGS_H

#include <string>

class CfgReader;

struct window_settings_t
{
    std::string title;
    int pos_x;
    int pos_y;
    int width;
    int height;
    int screen_number;
    int samples;
    bool fullscreen;
    bool vsync;
    bool double_buffer;

    window_settings_t();
    void read(CfgReader& cfg);
    void print_in_journal() const;
};

#endif // EDITOR_WINDOW_SETTINGS_H
