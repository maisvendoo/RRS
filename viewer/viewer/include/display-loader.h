#ifndef     DISPLAY_LOADER_H
#define     DISPLAY_LOADER_H

#include    "display-container.h"

#include    <osg/Node>

void loadDisplayModule(const display_config_t &display_config,
                        display_container_t *dc,
                        osg::Node *model);

#endif // DISPLAY_LOADER_H
