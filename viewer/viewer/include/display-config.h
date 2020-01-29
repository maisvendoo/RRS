#ifndef     DISPLAY_CONFIG_H
#define     DISPLAY_CONFIG_H

#include    <QString>

struct      display_config_t
{
    QString     module_name;
    QString     surface_name;

    display_config_t()
        : module_name("")
        , surface_name("")
    {

    }
};

#endif // DISPLAY_CONFIG_H
