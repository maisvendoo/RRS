//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     OBJECT_H
#define     OBJECT_H

#include    <QString>

struct object_t
{
    QString     name;
    QString     mode;
    QString     model_path;
    QString     texture_path;

    object_t()
        : name("")
        , mode("")
        , model_path("")
        , texture_path("")
    {

    }
};

#endif // OBJECT_H
