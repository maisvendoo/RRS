#include "editor/settings/WindowSettings.h"

#include <CfgReader.h>
#include <Journal.h>
#include <core/string_funcs.h>

#include <QString>

window_settings_t::window_settings_t()
    : title("Route Editor")
    , pos_x(50)
    , pos_y(50)
    , width(800)
    , height(600)
    , screen_number(0)
    , samples(1)
    , fullscreen(false)
    , vsync(false)
    , double_buffer(true)
{
}

void window_settings_t::read(CfgReader& cfg)
{
    const QString section = "Window";

    QString temp_qstring = to_qstring(title);
    if (cfg.getString(section, "Title", temp_qstring))
    {
        title = to_std_string(temp_qstring);
    }

    cfg.getInt(section, "PosX", pos_x);
    cfg.getInt(section, "PosY", pos_y);
    cfg.getInt(section, "Width", width);
    cfg.getInt(section, "Height", height);

    int temp_int = screen_number;
    cfg.getInt(section, "ScreenNumber", temp_int);
    if (temp_int >= 0)
    {
        screen_number = temp_int;
    }

    cfg.getInt(section, "Samples", samples);
    cfg.getBool(section, "FullScreen", fullscreen);
    cfg.getBool(section, "VSync", vsync);
    cfg.getBool(section, "DoubleBuffer", double_buffer);
}

void window_settings_t::print_in_journal() const
{
    Journal* const journal = Journal::instance();

    journal->debug("Window settings:");
    journal->debug("    title: " + to_qstring(title));
    journal->debug("    pos_x: " + QString::number(pos_x));
    journal->debug("    pos_y: " + QString::number(pos_y));
    journal->debug("    width: " + QString::number(width));
    journal->debug("    height: " + QString::number(height));
    journal->debug("    screen_number: " + QString::number(screen_number));
    journal->debug("    samples: " + QString::number(samples));
    journal->debug("    fullscreen: " + to_qstring(fullscreen));
    journal->debug("    vsync: " + to_qstring(vsync));
    journal->debug("    double_buffer: " + to_qstring(double_buffer));
}
