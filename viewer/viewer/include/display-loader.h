#ifndef     DISPLAY_LOADER_H
#define     DISPLAY_LOADER_H

#include    "display.h"

#include    <osg/Node>

AbstractDisplay *loadDisplayModule(const QString &module_path,
                                   const QString &surface_name,
                                   osg::Node *model);

#endif // DISPLAY_LOADER_H
