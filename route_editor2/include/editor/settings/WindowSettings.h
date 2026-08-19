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

    /**
     * @brief Construct a new window_settings_t object
     *        and initialize it with default values.
     */
    window_settings_t();

    /**
     * @brief Read the window settings from a config file.
     *
     * @param[in] cfg CfgReader, which is associated with a config
     *                file containing the editor settings.
     */
    void read(CfgReader& cfg);

    /**
     * @brief Print the window settings in the Journal.
     */
    void print_in_journal() const;
};

#endif // EDITOR_WINDOW_SETTINGS_H
