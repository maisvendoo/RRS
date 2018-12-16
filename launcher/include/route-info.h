#ifndef     ROUTEINFO_H
#define     ROUTEINFO_H

#include    <QString>

struct route_info_t
{
    QString route_dir;
    QString route_title;
    QString route_description;

    route_info_t()
        : route_dir("")
        , route_title("")
        , route_description("")
    {

    }
};

#endif // ROUTEINFO_H
