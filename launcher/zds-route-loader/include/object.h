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

#include    <osg/Geometry>

struct object_ref_t
{
    QString     name;
    QString     mode;
    QString     model_path;
    QString     texture_path;

    object_ref_t()
        : name("")
        , mode("")
        , model_path("")
        , texture_path("")
    {

    }
};

struct objects_dat_t
{
    QString     name;
    float       ordinate;
    float       shift;
    float       angle_horizontal;
    float       angle_vertical;
    float       height;
};

struct node_t
{
    QString     name;
    osg::Node   *node;

    node_t()
        : name("")
        , node(nullptr)
    {

    }
};

#endif // OBJECT_H
